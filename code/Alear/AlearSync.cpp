#include "AlearSync.h"
#include "AlearStartMenu.h"

#include <algorithm>
#include <set>
#include <json_ext.h>

#include <cell/fs/cell_fs_file_api.h>

#include <filepath.h>
#include <Directory.h>
#include <sha1.h>
#include <GuidHash.h>
#include <GuidHashMap.h>
#include <FartRO.h>
#include <MMString.h>
#include <ResourceSystem.h>
#include <ResourceGFXFont.h>
#include <LoadingScreen.h>
#include <network/NetworkManager.h>
#include <GetLoginCookieTask.h>
#include <View.h>
#include <OverlayUI.h>
#include <GFXApi.h>
#include <gooey/GooeyNodeManager.h>
#include <gooey/GooeyScriptInterface.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cell/thread.h>
#include <cell/DebugLog.h>
#include <System.h>

char* gSyncDatabasePath = "gamedata/alear/sync/alrs.map";
char* gSyncCachePath = "gamedata/alear/sync/alrs.farc";
char* gSyncPublishPath = "gamedata/alear/sync/publish";

bool AppendFileToCache(CFartRO* cache, char* data, size_t len, CHash& sha1);
bool IsHashInAnyCache(CHash& sha1)
{
    u32 size;
    for (int i = 0; i < CT_COUNT; ++i)
    {
        CCache* cache = gCaches[i];
        if (cache == NULL) continue;

        // If we can get the size, it means it's in the cache.
        if (cache->GetSize(sha1, size)) return true;
    }

    return false;
}

bool PreferLoose(const char* path)
{
    if (path == NULL) return false;

    const char* extensions[] = {".fev", ".fsb", ".mp3", ".fnt", ".bik", ".farc", ".per", ".osb", ".edat", ".sdat", ".map"};
    for (int i = 0; i < ARRAY_LENGTH(extensions); ++i)
    {
        if (strstr(path, extensions[i]) != NULL) 
            return true;
    }

    return false;
}

enum EPacketType {
    MLD_RESET = 0x60,
    MLD_KEEP_ALIVE,
    MLD_HANDSHAKE,
    MLD_ACK,
    MLD_NETCODE,
    MLD_GAMEDATA,
    MLD_RELIABLE
};

enum EGamedataFlags {
    E_LOCK_RESOURCE_SYSTEM = 0x1,
    E_UNLOCK_RESOURCE_SYSTEM = 0x2,
    E_RELOAD_DATABASE = 0x4,
    E_LOOSE_FILE = 0x8,
    E_EVENT = 0x10,
    E_FINISH_PUBLISH_SUCCESS = 0x20,
    E_FINISH_PUBLISH_FAIL = 0x40,
    E_FINISH_PUBLISH_NO_CHANGES = 0x80,
    E_DATABASE = 0x100
};

enum ENetcodeMessage {
    MLD_PRINT,
    MLD_COMMIT,
    MLD_REQUEST_RESOURCES,
    MLD_DELETE_RESOURCES,
    MLD_PUBLISH_HISTORY,
};

// Standard matchmaking procedures
// List all matches currently in progress, if none exist, just create one
// Broadcast session info that player has joined to everybody already in the session (our status is JOINING)
// Upload client owned sync objects, in this case just RSyncedProfile resources (allow crossgame?)
// Send all player data in the session to the person joining
// Send all sync object data in the session to the person joining
    // The RLevel resource is a sync object owned by the "host" that gets updated ONLY when a player joins
    // so they have a sync point to connect to.
// Once we've received all data and uploaded our own data, set our status to PLAYING

// This may make it difficult to handle any cheating, although realistically not an issue for small playerbase,
// but in asynchronous mode, input is synced, but player's will have authority over their own position.
// Lerped position + predictive fork for transform based on client input.


enum EGameRoomState {
    GAMEROOM_NONE,
    GAMEROOM_WAITING_MIN_PLAYERS,
    GAMEROOM_READY,
    GAMEROOM_DOWNLOADING,
    GAMEROOM_COUNTING_DOWN,
    GAMEROOM_COUNTING_DOWN_PAUSED,
    GAMEROOM_GAME_IN_PROGRESS
};

enum EGamedataMessage {
    MLD_SYNC_OBJECT_CREATE,
    MLD_SYNC_OBJECT_REMOVE,
    MLD_SYNC_OBJECT_UPDATE,
    
    MLD_PLAYER_INFO,
    MLD_PLAYER_STATE,
    MLD_PLAYER_JOIN,
    MLD_PLAYER_LEAVE,

    MLD_WORLD_OBJECT_CREATE,
    MLD_WORLD_OBJECT_STATE,

    MLD_SEED,

    MLD_ROOM_SETTINGS,
    MLD_CHANGE_HOST,
    MLD_KICK_PLAYER,

    MLD_GAMEPLAY,
    MLD_TEXT_CHAT_MESSAGE,
    MLD_VOIP_PACKET,
};

enum EConnectFlags {
    E_NONE = 0x0,
    E_ANONYMOUS = 0x1,
    E_NO_SYNC = 0x2
};

const u32 PACKET_HEADER_SIZE = 0x10;
const u32 PROTOCOL_VERSION = 0x2;
const u32 MAX_PAYLOAD_SIZE = 65535 - 16;
const u32 BUFFER_SIZE = 4096;

struct NetPacket {
    u8 Protocol;
    u8 Version;
    u16 Length;
    union {
        struct { u32 Message; } Netcode;
        struct { 
            u32 TicketFileSize;
            u32 Flags;
            u32 Permissions;
        } Handshake;
        struct {
            u32 Message;
            u32 SequenceNumber;
            u16 GroupNumber;
            u16 GroupSize;
        } Reliable;
        struct {
            u32 GroupIndex;
            u32 GroupSize;
        } Gamedata;

        u8 HeaderData[0xc];
    };
    char Payload[MAX_PAYLOAD_SIZE];
};

struct NetMessageRequestResources {
    u32 NumGuids;
    CGUID GUIDs[1];
};

struct NetMessageFile {
    char Filename[256];
    CHash FileHash;
    u32 FileSize;
    u32 Flags;
};

struct NetMessageCommit {
    char Author[20];
    u32 NumAdditions;
    u32 NumDeletes;
    u32 NumChanges;
};

struct NetMessagePublishHistory {
    SCommitData History[MAX_COMMIT_ENTRIES];
};

enum EClientState {
    STATE_DISCONNECTED,
    STATE_ESTABLISHED,
    STATE_SENT_HANDSHAKE,
    STATE_WAITING_FOR_HISTORY,
    STATE_WAITING_FOR_DATABASE,
    STATE_CONNECTED
};

s32 gSyncSocket = -1;
sockaddr_in gSyncAddr;
THREAD gSyncThread;
THREAD gUploadSyncThread;
NetPacket gRecvPacket;
NetPacket gSendPacket;
EClientState gClientState;
CFileDB* gSyncDatabase;

CCriticalSec gSocketWriteLock("SyncIO");
CCriticalSec gUploadLock("SyncUpload");
CFileDB* gDatabaseToUpload;

bool gResourceSystemLocked;
bool gWantCloseScreenHandle;
bool gWantConnectSyncServer;
void* gUploadScreenHandle;
SCommitData gCommitHistory[MAX_COMMIT_ENTRIES];
ESyncPermissions gSyncPermissions;
SDownloadInfo gDownloadInfo;

void TryConnectSyncServer()
{
    gWantConnectSyncServer = true;
}

ESyncPermissions GetSyncServerPermissions()
{
    return gSyncPermissions;
}

bool IsAlearSyncUploading()
{
    return gDatabaseToUpload != NULL || gUploadScreenHandle != NULL;
}

bool IsSyncServerConnecting()
{
    if (gClientState == STATE_CONNECTED) return false;
    return gWantConnectSyncServer || gClientState > STATE_DISCONNECTED;
}

void CloseDatabaseResponseScreen(CTextState* text, u32 result)
{
    gWantCloseScreenHandle = true;
}

void FinishUploadTask(u32 flags)
{
    if (gUploadScreenHandle != NULL)
        CancelActiveLoadingScreen(gUploadScreenHandle, false, 0);
    gUploadScreenHandle = NULL;

    const char* response = NULL;
    if (flags & E_FINISH_PUBLISH_FAIL) response = "Failed to publish database due to unknown error";
    if (flags & E_FINISH_PUBLISH_SUCCESS) response = "Publish succeeded";
    if (flags & E_FINISH_PUBLISH_NO_CHANGES) response = "Database contained no changes";

    if (response == NULL) return;

    CTextState state(0xc, 0, 0, 0x5, 0, NULL);
    MultiByteToTChar(state.title, "Alear Sync", NULL);
    MultiByteToTChar(state.text, response, NULL);
    gUploadScreenHandle = SetActiveLoadingScreen(&state, &CloseDatabaseResponseScreen, true);
}

void ClearDownloadInfo()
{
    memset(&gDownloadInfo, 0, sizeof(SDownloadInfo));
}

void MainThreadUpdate()
{
    if (gWantCloseScreenHandle)
    {
        if (gUploadScreenHandle != NULL)
            CancelActiveLoadingScreen(gUploadScreenHandle, false, 0);
        gUploadScreenHandle = NULL;
        gWantCloseScreenHandle = false;
    }
}

bool IsSyncServerConnected()
{
    return gClientState == STATE_CONNECTED;
}

void DeleteResources(CRawVector<CGUID>& files)
{
    if ((gSyncPermissions & PERMISSIONS_DELETE) == 0) return;
    if (files.size() == 0 || gClientState != STATE_CONNECTED) return;

    CCSLock _the_lock(&gSocketWriteLock, __FILE__, __LINE__);

    NetMessageRequestResources& msg = *(NetMessageRequestResources*)gSendPacket.Payload;
    msg.NumGuids = 0;

    // Compare entries to the database we currently have loaded in memory
    for (CGUID* guid = files.begin(); guid != files.end(); ++guid)
    {
        msg.GUIDs[msg.NumGuids++] = *guid;
    }

    gSendPacket.Protocol = MLD_NETCODE;
    gSendPacket.Version = PROTOCOL_VERSION;
    gSendPacket.Length = PACKET_HEADER_SIZE + sizeof(u32) + msg.NumGuids * sizeof(CGUID); 
    gSendPacket.Netcode.Message = MLD_DELETE_RESOURCES;
    send(gSyncSocket, (const void*)&gSendPacket, gSendPacket.Length, 0);
}

void UploadDatabase(CFileDB* database)
{
    if ((gSyncPermissions & PERMISSIONS_UPLOAD) == 0) return;
    if (database == NULL || gClientState != STATE_CONNECTED || IsAlearSyncUploading()) return;

    CTextState state(0xc, 0, 0, 0x1, 0, NULL);
    MultiByteToTChar(state.title, "Alear Sync", NULL);
    MultiByteToTChar(state.text, "Database sync in progress...", NULL);
    gUploadScreenHandle = SetActiveLoadingScreen(&state, NULL, true);

    CCSLock _the_lock(&gUploadLock, __FILE__, __LINE__);
    gDatabaseToUpload = database;
}

void CloseSocket()
{
    // If we crash while receiving gamedata, the resource system
    // will probably be locked, so make sure we unlock it again,
    // lest we get a fun deadlock.
    if (gResourceSystemLocked)
    {
        gResourceCS->Leave();
        gResourceSystemLocked = false;
    }

    ClearDownloadInfo();
    FinishUploadTask(0);
    gDatabaseToUpload = NULL;
    
    if (gSyncSocket == -1) return;
    socketclose(gSyncSocket);
    gSyncSocket = -1;
    gClientState = STATE_DISCONNECTED;
}

bool ReconnectSocket()
{
    if (gSyncSocket != -1) return true;
    if (!gWantConnectSyncServer) return false;

    gSyncSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(gSyncSocket, (sockaddr*)&gSyncAddr, sizeof(gSyncAddr)) < 0)
    {
        DebugLogChF(DC_MOLD, "Failed to establish connection to alear sync server!\n");
        CloseSocket();
        gWantConnectSyncServer = false;
        return false;
    }

    gWantConnectSyncServer = false;
    gClientState = STATE_ESTABLISHED;
    DebugLogChF(DC_MOLD, "Successfully connected to alear sync server!\n");

    return true;
}

ssize_t ReadExactly(int s, char* buf, ssize_t len, SDownloadInfo& info)
{
    info.TotalBytes = len;

    ssize_t n = 0;
    do
    {
        info.BytesReceived = n;
        int res = recv(s, buf + n, len - n, 0);
        if (res <= 0) return res;
        n += res;
    } while (n < len);

    return n;
}

ssize_t ReadExactly(int s, char* buf, ssize_t len)
{
    ssize_t n = 0;
    do
    {
        int res = recv(s, buf + n, len - n, 0);
        if (res <= 0) return res;
        n += res;
    } while (n < len);

    return n;
}

bool WriteStreamedFile(CFilePath& fp, u32 size, CHash& out_hash, SDownloadInfo& info)
{                    
    CSHA1Context sha1;
    FileHandle fd;

    info.TotalBytes = size;
    info.BytesReceived = 0;

    DirectoryCreate(fp);
    FileOpen(fp, &fd, OPEN_WRITE);

    char buf[BUFFER_SIZE];
    u32 remaining = size;
    while (remaining > 0)
    {
        ssize_t res = ReadExactly(gSyncSocket, buf, MIN(remaining, BUFFER_SIZE));
        if (res <= 0)
        {
            DebugLogChF(DC_MOLD, "Failed to write %s because server error occurred!\n", fp.c_str());
            CloseSocket();
            FileClose(&fd);

            return false;
        }

        sha1.AddData((const u8*)buf, res);
        FileWrite(fd, (void*)buf, res);
        remaining -= res;
        info.BytesReceived += res;
    }

    sha1.Result((u8*)&out_hash);
    FileClose(&fd);

    return true;
}

void OnDatabaseSynced()
{
    CCSLock _the_lock(&gSocketWriteLock, __FILE__, __LINE__);
    DebugLogChF(DC_MOLD, "Received database sync, calculating diffs...\n");

    CFilePath fp(FPR_GAMEDATA, gSyncDatabasePath);
    CFileDB* database = CFileDB::Construct(fp);
    database->Load();

    NetMessageRequestResources& msg = *(NetMessageRequestResources*)gSendPacket.Payload;
    msg.NumGuids = 0;

    // Compare entries to the database we currently have loaded in memory
    for (CFileDBRow* row = database->Files.begin(); row != database->Files.end(); ++row)
    {
        // ignoring any hashes that aren't set, this shouldn't even happen anyway
        if (!row->FileHash) continue;
        
        if (PreferLoose(row->FilePathX))
        {
            // probably a bad way to do comparisons, but since there's only a few
            // loose types, this is probably fine and faster than hashing the data.
            CFilePath loose_file(FPR_GAMEDATA, row->FilePathX);
            u64 modtime, size;

            if (!FileExists(loose_file) || (FileStat(loose_file, &modtime, &size) && size != row->FileSize))
            {
                msg.GUIDs[msg.NumGuids++] = row->FileGuid;
                DebugLogChF(DC_MOLD, "requesting loose file %s (g%d)...\n", row->FilePathX, row->FileGuid.guid);
            }
        }
        else if (!IsHashInAnyCache(row->FileHash))
        {
            msg.GUIDs[msg.NumGuids++] = row->FileGuid;
            DebugLogChF(DC_MOLD, "requesting %s (g%d)...\n", row->FilePathX, row->FileGuid.guid);
        }
    }

    // delete the database for now, loading databases is basically immediate,
    // and we don't actually need it at the moment
    delete database;

    gSendPacket.Protocol = MLD_NETCODE;
    gSendPacket.Version = PROTOCOL_VERSION;
    gSendPacket.Length = PACKET_HEADER_SIZE + sizeof(u32) + msg.NumGuids * sizeof(CGUID); 
    gSendPacket.Netcode.Message = MLD_REQUEST_RESOURCES;
    send(gSyncSocket, (const void*)&gSendPacket, gSendPacket.Length, 0);
}

void AlearSyncUploadThreadFunc(u64 arg)
{
    DebugLogChF(DC_MOLD, "Starting Alear Sync Upload Thread!\n");
    while (!WantQuitOrWantQuitRequested())
    {
        if (gDatabaseToUpload == NULL || gClientState != STATE_CONNECTED)
        {
            ThreadSleep(33);
            continue;
        }

        CCSLock _upload_lock(&gUploadLock, __FILE__, __LINE__);
        CCSLock _write_lock(&gSocketWriteLock, __FILE__, __LINE__);

        CFileDB* database = gDatabaseToUpload;
        gDatabaseToUpload = NULL;

        wchar_t fmt[512];
        int file_index = 0;
        for (CFileDBRow* row = database->Files.begin(); row != database->Files.end(); ++row)
        {
            gSendPacket.Protocol = MLD_GAMEDATA;
            gSendPacket.Version = PROTOCOL_VERSION;
            gSendPacket.Length = PACKET_HEADER_SIZE + sizeof(NetMessageFile);
            NetMessageFile& file = *((NetMessageFile*)gSendPacket.Payload);

            CFilePath fp(FPR_GAMEDATA, row->FilePathX);

            FileHandle fd;
            u32 size;

            FormatString<512>(fmt, L"Uploading %s... (%d of %d)", row->FilePathX, ++file_index, database->Files.size());
            ((CTextState*)gUploadScreenHandle)->text = (tchar_t*)fmt;

            SResourceReader reader;
            if (!row->FileHash || PreferLoose(row->FilePathX))
            {
                u64 modtime, filesize;
                if (!FileStat(fp, &modtime, &filesize)) continue;
                size = filesize;
                if (!FileOpen(fp, &fd, OPEN_READ)) continue;
            }
            else 
            {
                if (!GetResourceReader(row->FileHash, reader)) continue;
                size = reader.Size;
                fd = reader.Handle;
            }


            StringCopy<char, 256>(file.Filename, row->FilePathX);
            file.FileHash.Clear();
            file.Flags = 0;
            file.FileSize = size;

            send(gSyncSocket, (void*)&gSendPacket, gSendPacket.Length, 0);

            char buf[BUFFER_SIZE];
            while (size > 0)
            {
                u64 n = MIN(size, BUFFER_SIZE);
                FileRead(fd, (void*)buf, n);

                size -= n;

                send(gSyncSocket, (void*)buf, n, 0);
            }

            FileClose(&fd);
            reader.Handle = -1;
        }

        ((CTextState*)gUploadScreenHandle)->text = (tchar_t*)L"Syncing database file...";

        ByteArray b; CHash h;
        if (!FileLoad(database->Path, b, h)) return;

        gSendPacket.Protocol = MLD_GAMEDATA;
        gSendPacket.Version = PROTOCOL_VERSION;
        gSendPacket.Length = PACKET_HEADER_SIZE + sizeof(NetMessageFile);
        NetMessageFile& file = *((NetMessageFile*)gSendPacket.Payload);

        StringCopy<char, 256>(file.Filename, "AlearSyncDatabase");
        file.Flags = E_DATABASE;
        file.FileSize = b.size();

        send(gSyncSocket, (void*)&gSendPacket, gSendPacket.Length, 0);

        int offset = 0, size = file.FileSize;
        while (size > 0)
        {
            u64 n = MIN(size, BUFFER_SIZE);
            
            send(gSyncSocket, (void*)(b.begin() + offset), n, 0);

            offset += n;
            size -= n;
        }
        
        // thread owns the database delete it here
        delete database;
    }

    ExitPPUThread(0);
}

void AlearSyncThreadFunc(u64 arg)
{
    DebugLogChF(DC_MOLD, "Starting Alear Sync Thread!\n");    

    while (!WantQuitOrWantQuitRequested())
    {
        //while (!gNetworkManager.ConnectionManager.DownloadOK) ThreadSleep(33);

        if (!ReconnectSocket()) continue;
        if (gClientState == STATE_DISCONNECTED) continue;

        // Since we've just established a connection to the server,
        // send a handshake packet that contains our game's current state.
        if (gClientState == STATE_ESTABLISHED)
        {
            CCSLock _the_lock(&gSocketWriteLock, __FILE__, __LINE__);
            CCSLock _cookie_lock(&gCookieSec, __FILE__, __LINE__);

            gSendPacket.Protocol = MLD_HANDSHAKE;
            gSendPacket.Version = PROTOCOL_VERSION;
            gSendPacket.Length = PACKET_HEADER_SIZE;
            gSendPacket.Handshake.TicketFileSize = 0;
            gSendPacket.Handshake.Permissions = PERMISSIONS_NONE;
            gSendPacket.Handshake.Flags = E_NONE;

            if (GetNPTicket())
            {
                gSendPacket.Length += gNPTicket.size();
                gSendPacket.Handshake.TicketFileSize = gNPTicket.size();

                send(gSyncSocket, (const void*)&gSendPacket, PACKET_HEADER_SIZE, 0);

                int offset = 0, size = gNPTicket.size();
                while (size > 0)
                {
                    u64 n = MIN(size, BUFFER_SIZE);
                    
                    send(gSyncSocket, (void*)(gNPTicket.begin() + offset), n, 0);

                    offset += n;
                    size -= n;
                }
            }
            else
            {
                gSendPacket.Handshake.Flags |= E_ANONYMOUS;
                send(gSyncSocket, (const void*)&gSendPacket, PACKET_HEADER_SIZE, 0);
            }

            gClientState = STATE_SENT_HANDSHAKE;
        }

        ssize_t len = ReadExactly(gSyncSocket, (char*)&gRecvPacket, PACKET_HEADER_SIZE);
        if (len <= 0)
        {
            DebugLogChF(DC_MOLD, "Disconnected from server!\n");
            CloseSocket();
            continue;
        }

        if (len != PACKET_HEADER_SIZE)
        {
            DebugLogChF(DC_MOLD, "Received packet with invalid header size, disconnecting!\n");
            CloseSocket();
            continue;
        }

        if (gRecvPacket.Version != PROTOCOL_VERSION)
        {
            DebugLogChF(DC_MOLD, "Server version (v%d) is compatible with client version (v%d)!\n", gRecvPacket.Version, PROTOCOL_VERSION);
            CloseSocket();
            continue;
        }

        // Length of the packet includes the header
        u16 size = gRecvPacket.Length - 16;

        if (size != 0)
        {
            len = ReadExactly(gSyncSocket, (char*)gRecvPacket.Payload, size);
            if (len != size)
            {
                DebugLogChF(DC_MOLD, "Not enough data in stream for message! Disconnecting!\n");
                CloseSocket();
                continue;
            }
        }

        // Wait for the server's handshake response before doing anything else,
        // we technically should receive this immediately after, but you know, just
        // in case do some checking.
        if (gClientState == STATE_SENT_HANDSHAKE)
        {
            if (gRecvPacket.Protocol != MLD_HANDSHAKE)
            {
                DebugLogChF(DC_MOLD, "Expected handshake response packet! Disconnecting!\n");
                CloseSocket();
                continue;
            }

            gSyncPermissions = (ESyncPermissions)gRecvPacket.Handshake.Permissions;

            if ((gRecvPacket.Handshake.Flags & E_NO_SYNC) == 0)
            {
                DebugLogChF(DC_MOLD, "Finished handshake, waiting for database sync...\n");
                gClientState = STATE_WAITING_FOR_HISTORY;
            }
            else DebugLogChF(DC_MOLD, "Finished handshake\n");

            continue;
        }
        else if (gClientState == STATE_WAITING_FOR_HISTORY)
        {
            if (gRecvPacket.Protocol != MLD_NETCODE || gRecvPacket.Netcode.Message != MLD_PUBLISH_HISTORY)
            {
                DebugLogChF(DC_MOLD, "Expected publish history message! Disconnecting!\n");
                CloseSocket();
                continue;
            }

            // Assuming success beforehand, if there's any actual error from here on out, it'll
            // just disconnect the socket anyway.
            gClientState = STATE_WAITING_FOR_DATABASE;
        }
        else if (gClientState == STATE_WAITING_FOR_DATABASE)
        {
            if (gRecvPacket.Protocol != MLD_GAMEDATA || (((NetMessageFile*)gRecvPacket.Payload)->Flags & E_DATABASE) == 0)
            {
                DebugLogChF(DC_MOLD, "Expected database sync message! Disconnecting!\n");
                CloseSocket();
                continue;
            }

            gClientState = STATE_CONNECTED;
        }
        // Everything past this point assumes the handshake has succeeded and we're
        // fully connected to the server.
        else if (gClientState != STATE_CONNECTED) continue;

        switch (gRecvPacket.Protocol)
        {
            case MLD_GAMEDATA:
            {
                NetMessageFile& file = *((NetMessageFile*)gRecvPacket.Payload);

                // if ((file.Flags & E_LOCK_RESOURCE_SYSTEM) != 0)
                // {
                //     DebugLogChF(DC_MOLD, "Attempting to lock resource system!\n");
                //     if (!gResourceSystemLocked)
                //     {
                //         gResourceSystemLocked = true;
                //         gResourceCS.Enter(__FILE__, __LINE__);
                //     }
                // }

                if ((file.Flags & E_FINISH_PUBLISH_FAIL) != 0)
                    FinishUploadTask(file.Flags);
                if ((file.Flags & E_FINISH_PUBLISH_NO_CHANGES) != 0)
                    FinishUploadTask(file.Flags);
                if ((file.Flags & E_FINISH_PUBLISH_SUCCESS) != 0)
                    FinishUploadTask(file.Flags);

                if ((file.Flags & E_DATABASE) != 0)
                    StringCopy<char, 256>(file.Filename, gSyncDatabasePath);

                if ((file.Flags & E_EVENT) == 0)
                {
                    ClearDownloadInfo();

                    gDownloadInfo.FileNumber = gRecvPacket.Gamedata.GroupIndex;
                    gDownloadInfo.FileTotal = gRecvPacket.Gamedata.GroupSize;
                    gDownloadInfo.TotalBytes = file.FileSize;
                    StringCopy<char, 256>(gDownloadInfo.Filename, file.Filename);

                    char hex[HASH_HEX_STRING_LENGTH];
                    file.FileHash.ConvertToHex(hex);
                    DebugLogChF(DC_MOLD, "Downloading \"%s\" (h%s) [FileSize]=0x%08x\n", file.Filename, hex, file.FileSize);

                    if (file.Flags & E_LOOSE_FILE)
                    {
                        CFilePath fp(FPR_GAMEDATA, file.Filename);
                        CHash computed;
                        if (!WriteStreamedFile(fp, file.FileSize, computed, gDownloadInfo)) break;

                        char chex[HASH_HEX_STRING_LENGTH];
                        computed.ConvertToHex(chex);

                        DebugLogChF(DC_MOLD, "Downloaded %s, computed SHA1 is %s\n", file.Filename, chex);
                    }
                    else
                    {
                        ByteArray b;
                        b.try_resize(file.FileSize);

                        if (ReadExactly(gSyncSocket, b.begin(), file.FileSize, gDownloadInfo) <= 0)
                        {
                            DebugLogChF(DC_MOLD, "Failed to download %s, since an error occurred reading stream!\n", file.Filename);
                            break;
                        }

                        AppendFileToCache((CFartRO*)gCaches[CT_SYNC], b.begin(), b.size(), file.FileHash);
                    }

                    ClearDownloadInfo();
                }

                if ((file.Flags & E_DATABASE) != 0)
                    OnDatabaseSynced();
                
                bool reload = (file.Flags & E_RELOAD_DATABASE) != 0;

                CFileDB* old_database = NULL;
                CFileDB* new_database = NULL;
                if (reload)
                {
                    DebugLogChF(DC_MOLD, "Reloading database!\n");

                    CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
                    
                    // The first database is always the sync database
                    old_database = FileDB::DBs[0];

                    // Load the sync map again!
                    CFilePath syncfp(FPR_GAMEDATA, gSyncDatabasePath);
                    new_database= CFileDB::Construct(syncfp);
                    new_database->Load();
                    FileDB::DBs[0] = new_database;
                }

                // if ((file.Flags & E_UNLOCK_RESOURCE_SYSTEM) != 0)
                // {
                //     DebugLogChF(DC_MOLD, "Unlocking resource system!\n");
                //     if (gResourceSystemLocked)
                //     {
                //         gResourceCS.Leave();
                //         gResourceSystemLocked = false;
                //     }
                // }

                if (reload && new_database != NULL)
                {
                    ReloadModifiedResources(new_database, old_database);
                    if (old_database != NULL) delete old_database;
                    NotifyStartMenu(NOTIFICATION_REFRESH_UI | NOTIFICATION_RELOAD_FILESYSTEM);
                }
                
                break;
            }
            case MLD_NETCODE:
            {
                DebugLogChF(DC_MOLD, "Received MLD_NETCODE packet w/ size=%d, type=%d\n", size, gRecvPacket.Netcode.Message);
                switch (gRecvPacket.Netcode.Message)
                {
                    case MLD_PRINT:
                    {
                        DebugLogChF(DC_MOLD, "MLD_PRINT: %s\n", gRecvPacket.Payload);
                        break;
                    }
                    case MLD_PUBLISH_HISTORY:
                    {
                        memcpy((void*)gCommitHistory, (void*)gRecvPacket.Payload, sizeof(gCommitHistory));
                        NotifyStartMenu(NOTIFICATION_REFRESH_UI);
                        break;
                    }
                    default:
                    {
                        DebugLogChF(DC_MOLD, "Received MLD_NETCODE packet with unknown message type! (%08x) Disconnecting client!\n", gRecvPacket.Netcode.Message);
                        CloseSocket();
                        break;
                    }
                }

                break;
            }
            default:
            {
                DebugLogChF(DC_MOLD, "Received unknown packet type! (%08x) Disconnecting client!\n", gRecvPacket.Protocol);
                CloseSocket();
                break;
            }
        }
    }

    CloseSocket();
    ExitPPUThread(0);
}

bool InitAlearSync()
{
    CFilePath fp(FPR_GAMEDATA, "gamedata/alear/config.json");

    // I suppose just consider it a success and just don't start the server?
    if (!FileExists(fp)) return true;

    char* json = FileLoadText(fp);
    if (json == NULL)
    {
        DebugLog("Failed to read configuration file at %s!\n", fp.c_str());
        return false;
    }

    #define FREE_AND_RETURN(value) delete json; return value;

    const int MAX_FIELDS = 32;
    json_t pool[MAX_FIELDS];
    json_t const* root = json_create(json, pool, MAX_FIELDS);
    if (root == NULL)
    {
        DebugLog("Failed to parse configuration for sync server!\n");
        FREE_AND_RETURN(false);
    }

    if (json_getType(root) != JSON_OBJ)
    {
        DebugLog("Root of configuration must be a JSON object!\n");
        FREE_AND_RETURN(false);
    }

    // If the server isn't enabled, we can just early return here.
    if (!json_getBoolProperty(root, "syncServerEnabled", false))
    {
        FREE_AND_RETURN(true);
    }

    const char* address = json_getPropertyValue(root, "syncServerAddress");
    if (address == NULL)
    {
        DebugLog("No server address specified for sync server!\n");
        FREE_AND_RETURN(false);
    }

    int port = json_getIntProperty(root, "syncServerPort", 16723);

    memset(&gSyncAddr, 0, sizeof(sockaddr_in));
    gSyncAddr.sin_family = AF_INET;
    gSyncAddr.sin_port = htons(port);
    inet_pton(AF_INET, address, &gSyncAddr.sin_addr.s_addr);

    gWantConnectSyncServer = true;
    gSyncThread = CreatePPUThread(&AlearSyncThreadFunc, NULL, "AlearSync", 3000, 0x10000, 1);
    gUploadSyncThread = CreatePPUThread(&AlearSyncUploadThreadFunc, NULL, "AlearSync Uploader", 3000, 0x10000, 1);

    FREE_AND_RETURN(true);
    #undef FREE_AND_RETURN
}

bool AppendFileToCache(CFartRO* cache, char* data, size_t len, CHash& sha1)
{
    CCSLock _the_lock(&cache->Mutex, __FILE__, __LINE__);

    CFartRO::CFAT fat;
    fat.hash = sha1;
    fat.size = len;
    fat.offset = 0;

    for (CFartRO::CFAT* it = cache->FAT.begin(); it != cache->FAT.end(); ++it)
    {
        fat.offset = MAX(fat.offset, it->offset + it->size);

        // Hash is already in the FAT, we can just ignore this data.
        if (it->hash == sha1) return true;
    }

    FileHandle fd;
    if (!FileOpen(cache->fp, &fd, OPEN_RDWR)) return false;

    // We probably have to sort the FAT?
    cache->FAT.push_back(fat);

    std::sort(cache->FAT.begin(), cache->FAT.end(), std::less<CFartRO::CFAT>());

    Footer footer;
    footer.count = cache->FAT.size();
    footer.magic = FARC;

    FileSeek(fd, fat.offset, CELL_FS_SEEK_SET);
    FileWrite(fd, (void*)data, len);
    FileWrite(fd, (void*)cache->FAT.begin(), cache->FAT.size() * sizeof(CFartRO::CFAT));
    FileWrite(fd, (void*)&footer, sizeof(Footer));
    FileClose(&fd);

    return true;
}

CGooeyNodeManager* gDownloadGooey;

void UpdateDownloadUI(CGooeyNodeManager* manager, SDownloadInfo& info)
{
    manager->SetStylesheetScalingFactor(0.5f);

    if (!manager->StartFrameNamed(0x444F574E4C4F4144ull)) return;
    

    manager->SetFrameSizing(gResX, gResY);
    manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_END);

    if (manager->StartFrame())
    {
        manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
        manager->SetFrameSizing(0.0f, 100.0f);
        manager->SetFrameBorders(16.0f, 32.0f);

        wchar_t text[512];
        FormatString<512>(text, L"Downloading %d of %d", info.FileNumber + 1, info.FileTotal);
        manager->DoText(text, GTS_T5i);
        manager->DoBreak();

        char* filename = strrchr(info.Filename, '/');
        if (filename == NULL) filename = info.Filename;
        else filename = filename + 1;

        FormatString<512>(text, L"%s", filename);

        manager->DoScrollingTextChangerNamed(123, text, GTS_T5i, v2(250.0f, 32.0f));
        manager->DoBreak();

        if (manager->StartFrame())
        {
            manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
            manager->SetFrameSizing(250.0f, 5.0f);
            manager->SetFrameBorders(0.0f, 0.0f);

            float progress = (float)info.BytesReceived / (float)info.TotalBytes;
            float remaining = 1.0f - progress;

            manager->DoRectangle(v2(progress * 250.0f, 5.0f, 0.0f, 1.0f), v4(1.0f, 0.0f, 0.0f, 1.0f));
            manager->DoRectangle(v2(remaining * 250.0f, 5.0f, 0.0f, 1.0f), v4(1.0f));

            manager->EndFrame();
        }


        manager->EndFrame();
    }



    manager->EndFrame();
}

bool gRenderThisFrame = false;
void UpdateDownloadInfo()
{
    if (gDownloadGooey == NULL)
        gDownloadGooey = new CGooeyNodeManager(INPUT_NONE, E_GOOEY_MANAGER_NETWORK_NONE);

    SDownloadInfo info = gDownloadInfo;
    gRenderThisFrame = StringLength(info.Filename) > 0;
    if (gRenderThisFrame)
    {
        UpdateDownloadUI(gDownloadGooey, info);
        gDownloadGooey->PerFrameUpdate(E_PLAYER_NUMBER_NONE, MODE_NORMAL, -1);
    }
}

void RenderDownloadInfo()
{
    if (gRenderThisFrame)
        gDownloadGooey->RenderToTexture(gResX, gResY, gResX, gResY, false, false);
}

void CloseAlearSync()
{

}