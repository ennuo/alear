#include <thread.h>
#include <ResourceSystem.h>
#include <SerialisedResource.h>
#include <ResourceTypes.h>
#include <Sync/Client.h>
#include <GuidHashMap.h>
#include <network/NetworkManager.h>
#include <System.h>
#include <Fart.h>
#include <Clock.h>

CSRQueue CSRsForSync;

CResource* _FindResourceInList(const CSerialisedResource* csr)
{
    CCSLock _lock(gResourceCS, __FILE__, __LINE__);
    CResource* rv = FindResourceInList(csr->GetDescriptor());
    if (rv == NULL || rv->CSR != csr)
    {
        for (CResource** itr = gResourceArray.begin(); itr != gResourceArray.end(); ++itr)
        {
            rv = *itr;
            if (rv->CSR == csr)
                return rv;
        }

        return NULL;
    }

    return rv;

}

MAKE_THREAD_FUNCTION(MainSlowThread)
{
    while (!CSRsForSlow->Aborted())
    {
        CP<CSerialisedResource> csr;
        int priority;

        if (!CSRsForSlow->Pop(priority, csr, -1)) 
            continue;

        if (WantQuit())
        {
            SetResourceError(csr, (EResourceLoadState)REFLECT_APPLICATION_QUITTING);
            continue;
        }

        CResource* r = _FindResourceInList(csr);
        CHash* loaded_hash = r != NULL ? &r->GetLoadedHash() : NULL;

        bool has_source = false;
        if (csr->LoosePath.IsEmpty())
        {
            SResourceReader reader;
            if (GetResourceReader(csr->GetDescriptor(), reader, csr->LoosePath))
            {
                has_source = FileLoad(reader, csr->Data);
                if (has_source && loaded_hash)
                    reader.RollingHash.Result((u8*)loaded_hash);
            }
        }
        else has_source = FileLoad(csr->LoosePath, csr->Data, loaded_hash);

        if (!has_source)
        {
            SetResourceError(csr, LOAD_STATE_ERROR_FILENOTFOUND);
        }
        else
        {
            AddCSRToDoneQueue(csr, priority);
        }
    }
}

MAKE_THREAD_FUNCTION(MainLoadingThread)
{
    while (!CSRsForStaging->Aborted())
    {
        CP<CSerialisedResource> csr;
        int priority;

        if (!CSRsForStaging->Pop(priority, csr, -1)) 
            continue;
        
        if (WantQuit())
        {
            SetResourceError(csr, (EResourceLoadState)REFLECT_APPLICATION_QUITTING);
            continue;
        }

        bool has_source = false;
        CCache* last_cache = NULL;

        CHash latest = csr->GetDescriptor().LatestHash();
        StringifyHash bytes(latest);

        ESerialisationType st = GetPreferredSerialisationType(csr->GetDescriptor().GetType());
        while (!WantQuit())
        {
            SResourceReader reader;
            if (!GetResourceReader(csr->GetDescriptor(), reader, csr->LoosePath))
                break;

            if (st == PREFER_FILE)
            {
                if (!csr->LoosePath.IsEmpty())
                {
                    has_source = true;
                    AddCSRToDoneQueue(csr, priority);
                }

                break;
            }
            else if (reader.Owner != NULL && reader.Owner->IsSlow(reader))
            {
                FileClose(reader);
                AddCSRToQueue(csr, CSRsForSlow, priority);
                has_source = true;
                break;
            }

            if (FileLoad(reader, csr->Data))
            {
                has_source = true;
                AddCSRToDoneQueue(csr, priority);
                break;
            }

            if ((last_cache != NULL && last_cache == reader.Owner) || has_source) break;

            MMLogCh(DC_RESOURCE, "************ resource cache gave us bad bytes (%s). trying again\n", bytes.c_str());
            last_cache = reader.Owner;
        }

        if (!has_source && !csr->LoosePath.IsEmpty())
        {
            has_source = true;
            AddCSRToQueue(csr, CSRsForSlow, priority);
        }

        if (!has_source)
        {
            if (sync::Client != NULL)
                AddCSRToQueue(csr, &CSRsForSync, priority);
            else
            {
                MMLogCh(DC_RESOURCE, "%f: Load from network fail, not ready :(\n", GetClockSeconds());
                MMLog("RESOURCE ERROR - LOAD_STATE_ERROR_NO_DATA_SOURCE\n");
                SetResourceError(csr, LOAD_STATE_ERROR_NO_DATA_SOURCE);
            }
        }
    }

    MMLogCh(DC_RESOURCE, "Leaving main load thread\n");
    THREAD_RETURN(0);
}

MAKE_THREAD_FUNCTION(SyncLoadThread)
{
    while (!CSRsForSync.Aborted())
    {
        CP<CSerialisedResource> csr;
        int priority;

        if (!CSRsForSync.Pop(priority, csr, -1)) 
            continue;

        SYNC_LOG("popped g%d from csr queue\n", csr->GetDescriptor().GetGUID().guid);

        sync::Client->Download(csr, priority);
    }

    THREAD_RETURN(0);
}
