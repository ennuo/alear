#include <thread.h>
#include <ResourceSystem.h>
#include <SerialisedResource.h>
#include <ResourceTypes.h>
#include <Sync/Client.h>
#include <GuidHashMap.h>
#include <network/NetworkManager.h>
#include <System.h>

CSRQueue CSRsForSync;

MAKE_THREAD_FUNCTION(SyncLoadThread)
{
    while (!CSRsForSync.Aborted())
    {
        CP<CSerialisedResource> csr;
        int priority;

        if (!CSRsForSync.Pop(priority, csr, -1)) 
            continue;

        SYNC_LOG("popped g%d from csr queue\n", csr->GetDescriptor().GetGUID().guid);

        // sync::Client->Download(csr, priority);
    }

    THREAD_RETURN(0);
}
