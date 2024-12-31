#ifndef ALEAR_SYNC_H
#define ALEAR_SYNC_H

#include <vector.h>
#include <MMString.h>
#include <GuidHash.h>

class CFileDB;
extern CFileDB* gSyncDatabase;

enum ESyncPermissions {
    PERMISSIONS_NONE = 0,
    PERMISSIONS_READ = 1,
    PERMISSIONS_UPLOAD = 2,
    PERMISSIONS_DELETE = 4
};

bool InitAlearSync();

void UploadDatabase(CFileDB* database);
void DeleteResources(CRawVector<CGUID>& files);

bool IsSyncServerConnected();
bool IsSyncServerConnecting();
ESyncPermissions GetSyncServerPermissions();
void TryConnectSyncServer();

void MainThreadUpdate();

extern char* gSyncDatabasePath;
extern char* gSyncCachePath;
extern char* gSyncPublishPath;

extern char gDownloadingFilename[256];
extern int gDownloadPercentage;

#define MAX_COMMIT_ENTRIES (10)

struct SCommitData
{
    char Author[20];
    int Additions, Deletes, Changes;
    char PublishDate[32];
};

struct SDownloadInfo {
    char Filename[256];
    int FileNumber;
    int FileTotal;
    int TotalBytes;
    int BytesReceived;
};


extern SDownloadInfo gDownloadInfo;
extern SCommitData gCommitHistory[MAX_COMMIT_ENTRIES];
extern char gCurrentSyncDownloadFilename[256];
void RenderDownloadInfo();
void UpdateDownloadInfo();


#endif // ALEAR_SYNC_H