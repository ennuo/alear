#include <Sync/Bootstrap.h>
#include <Sync/Client.h>
#include <Sync/Shared.h>

#include <filepath.h>
#include <Directory.h>

#include <Fart.h>
#include <FartRO.h>

extern MAKE_THREAD_FUNCTION(SyncLoadThread);

namespace sync
{
    int gServerPort = kSyncServerPort;
    const char* gServerAddress = "127.0.0.1";
    bool gServerEnabled = true;

    static THREAD ResourceThread;

    bool Open()
    {
        SYNC_LOG("server enabled: %s, server url: %s, server port: %d\n", gServerEnabled ? "true" : "false", gServerAddress, gServerPort);
        
        DirectoryCreate(CFilePath(FPR_GAMEDATA, "gamedata/alear/sync/publish/"));
        DirectoryCreate(CFilePath(FPR_GAMEDATA, "gamedata/alear/sync/local/"));
        DirectoryCreate(CFilePath(FPR_GAMEDATA, "gamedata/alear/sync/depots/"));

        const CFilePath cache_path(FPR_GAMEDATA, "gamedata/alear/sync/resourcecache.farc");
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

        gCaches[CT_SYNC] = MakeROCache(CFilePath(FPR_GAMEDATA, "gamedata/alear/sync/resourcecache.farc"), true, false);
        
        if (gServerEnabled)
        {
            DownloadJobManager = new CJobManager(kMaxConcurrentDownloads, "alear sync download job worker");
            ResourceThread = ThreadCreate(&SyncLoadThread, NULL, "alear sync loading thread");
            sync::Client = new sync::CClient();
            
            // wait for initial server bringup
            while (sync::Client->IsConnecting()) ThreadSleep(33);
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