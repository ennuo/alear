#include <Sync/Bootstrap.h>
#include <Sync/Client.h>
#include <Sync/Shared.h>
#include <Sync/Cache.h>
#include <Sync/Serialise.h>

#include <filepath.h>
#include <Directory.h>

#include <Fart.h>
#include <FartRO.h>
#include <ReadINI.h>
#include <FileWatcher.h>
#include <AlearStartMenu.h>

extern MAKE_THREAD_FUNCTION(SyncLoadThread);

namespace sync
{
    CIniSettings Config;
    CSyncDatabase Database;
    CVector<depot> Depots;
    CCriticalSec DepotMutex("DepotMutex");

    static THREAD ResourceThread;

    CFilePath GetDepotCacheFilePath()
    {
        return CFilePath(FPR_ALEAR, "sync/depotcache");
    }

    void LinkDepots()
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            depot& d = Depots[i];
            if (d.Database == NULL)
            {
                d.Database = new CFileDB(d.MakeDatabaseFilePath());
                d.Database->Load();
            }
        }
    }

    void DestroyDepotCache()
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);
        FileUnlink(GetDepotCacheFilePath());
    }

    void LoadDepotCache()
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);
        Depots.resize(0);

        ByteArray b;
        if (!FileLoad(GetDepotCacheFilePath(), b)) return;

        CReflectionLoadVector r(&b);
        r.SetCompressionFlags(0);
        u32 version;
        if (Reflect(r, version) != REFLECT_OK || version > kProtocolVersion)
        {
            DestroyDepotCache();
            return;
        }

        r.SetRevision(SRevision(version));
        if (Reflect(r, Depots) != REFLECT_OK)
        {
            Depots.resize(0);
            DestroyDepotCache();
        }
    }

    void SaveToDepotCache()
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);

        CSyncSaveVector r;
        u32 version = kProtocolVersion;
        if (Reflect(r, version) != REFLECT_OK) return;
        if (Reflect(r, Depots) != REFLECT_OK) return;

        int fd;
        if (FileOpen(GetDepotCacheFilePath(), fd, OPEN_WRITE))
        {
            FileWrite(fd, r.Data.begin(), r.Data.size());
            FileClose(fd);
        }
    }

    bool Open()
    {
        DirectoryCreate(CFilePath(FPR_ALEAR, "sync/publish/"));
        DirectoryCreate(CFilePath(FPR_ALEAR, "sync/depots/local"));
        DirectoryCreate(CFilePath(FPR_ALEAR, "sync/depots/remote"));

        const CFilePath cache_path(FPR_ALEAR, "sync/resourcecache.farc");
        if (!FileExists(cache_path))
        {
            Footer footer = { 0, FARC };
            int fd;
            if (FileOpen(cache_path, fd, OPEN_WRITE))
            {
                FileWrite(fd, &footer, sizeof(footer));
                FileClose(fd);
            }
        }

        gCaches[CT_SYNC] = new CMutableCache(cache_path);

        LoadDepotCache();
        LinkDepots();

        Config.ReadIniFile(CFilePath(FPR_ALEAR, "config/sync.ini"));
        if (Config.GetBool("enabled", true))
        {
            DownloadJobManager = new CJobManager(kMaxConcurrentDownloads, "alear sync download job worker");
            ResourceThread = ThreadCreate(&SyncLoadThread, NULL, "alear sync loading thread");
            sync::Client = new sync::CClient();
            
            // wait for initial server bringup
            while (sync::Client->IsConnecting()) ThreadSleep(500);

            if (sync::Client->IsConnected())
            {
                MMLog("CONNECTED!!!!!\n");
                sync::Client->DoUploadTest();
            }
            else
            {

            }

        }
        
        return true;
    }

    void Update()
    {
        if (sync::Client != NULL)
            sync::Client->UpdateSynced();
    }

    void Close()
    {
        if (ResourceThread != NULL)
            ThreadJoin(ResourceThread);
        ResourceThread = NULL;
    }


}