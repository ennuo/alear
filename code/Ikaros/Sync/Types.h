#pragma once

#include <vector.h>
#include <MMString.h>
#include <CalendarTime.h>
#include <GuidHash.h>
#include <GuidHashMap.h>
#include <network/NetworkUtilsNP.h>
#include <filepath.h>

namespace sync
{
    struct token
    {
        char bytes[32];
    };

    enum Permissions
    {
        ePermissions_Admin = (1 << 0),
        ePermissions_Download = (1 << 1),
        ePermissions_Upload = (1 << 2),
        ePermissions_ManageDepots = (1 << 3),
        ePermissions_ReadDepots = (1 << 4)
    };

    enum GateEventType
    {
        eEventType_PlayerConnected = 0x80001000,
        eEventType_PlayerDisconnected
    };

    enum SyncEventType
    {
        eEventType_DepotCreate = 0x80101000,
        eEventType_DepotUpdate,
        eEventType_DepotDelete,
        eEventType_Commit
    };

    enum CrafteroidEventType
    {
        eEventType_PlanetInfoUpdated = 0x80201000,
        eEventType_PlanetDecorationsUpdated,
        eEventType_LevelCreated,
        eEventType_LevelDeleted,
        eEventType_LevelUpdated
    };

    enum GateMessageType
    {
        eMessageType_ServerInfo = 0x80000000,
        eMessageType_Login,
        eMessageType_UserInfo,
    };

    enum ResourceMessageType
    {
        eMessageType_FilterResources = 0x80300000,
        eMessageType_UploadResource,
        eMessageType_Download
    };

    enum SyncMessageType
    {
        eMessageType_DepotList = 0x80100000,
        eMessageType_Depot,
        eMessageType_Database,
        eMessageType_RecentActivity,
        eMessageType_CommitInfo
    };

    enum CrafteroidMessageType
    {
        eMessageType_GetPlanetList = 0x80200000,
        eMessageType_GetLevelList,
        eMessageType_UpdatePlanetDecorations,
        eMessageType_AddEmptySlot,
        eMessageType_PublishSlot,
        eMessageType_DeleteSlot
    };

    enum AuthenticationType
    {
        eAuthenticationType_Ticket,
        eAuthenticationType_Credentials
    };

    enum ConnectedClient
    {
        eClientType_PS3,
        eClientType_PSV,
        eClientType_PC,
        eClientType_RPCS3
    };
    
    enum
    {
        eChannel_Gate = 0x67617465 /* gate */,
        eChannel_Resource = 0x72737263, /* rsrc */
        eChannel_Sync = 0x73796e63, /* sync */
        eChannel_Crafteroids = 0x63726674 /* crft */
    };

    enum
    {
        eMethod_Request,
        eMethod_Create,
        eMethod_Update,
        eMethod_Delete,
        eMethod_Event,
        eMethod_Response
    };

    enum
    {
        eDepotFlags_Local = (1 << 0)
    };
    
    struct packet
    {
        u32 Length;
        u32 Message;
        u32 Channel;
        u16 Method;
        u16 Status;
        u32 Mark;
    };

    struct extension_info
    {
        inline extension_info() : ProtocolVersion(), Id(), MessageChannel()
        {
        }

        u32 ProtocolVersion;
        u32 MessageChannel;
        MMString<char> Id;
    };

    struct SServerInfo 
    {
        inline SServerInfo() : ProtocolVersion(),
        Name(), Version(), BuildDate(), Extensions()
        {

        }

        u32 ProtocolVersion;
        MMString<char> Name;
        MMString<char> Version;
        MMString<char> BuildDate;
        CVector<extension_info> Extensions;
    };

    struct depot
    {
        inline depot() : DepotID(), CommitID(), Name(), Id(), Priority(), Database(), Flags()
        {

        }
        
        inline bool IsLocal() const
        {
            return (Flags & eDepotFlags_Local) != 0;
        }

        inline bool IsRemote() const
        {
            return (Flags & eDepotFlags_Local) == 0;
        }
        
        inline CFilePath MakeDatabaseFilePath() const
        {
            CFilePath fp(FPR_GAMEDATA, IsLocal() ? "gamedata/alear/sync/depots/local" : "gamedata/alear/sync/depots/remote");
            fp.Append(Id);
            fp.AppendRaw(".map");

            return fp;

        }

        u64 DepotID;
        u64 CommitID;
        char Name[128];
        char Id[32];
        u32 Priority;
        CFileDB* Database;
        u32 Flags;
    };

    struct SSerializedCrafteroid
    {
        inline SSerializedCrafteroid() : RemoteId(), OwnerId(), Name(), PlanetDecorations()
        {

        }
        
        u64 RemoteId;
        u64 OwnerId;
        MMString<char> Name;
        MMString<char> PlanetDecorations;
    };

    class CFileSource {
    public:
        inline CFileSource() : Hash(), FilePath() {}
        inline CFileSource(const CHash& hash) : Hash(hash), FilePath() {}
        inline CFileSource(const CFilePath& fp) : Hash(), FilePath()
        {
            if (FileHash(fp, &Hash))
                FilePath = fp;
        }
    public:
        inline bool IsValid() const { return Hash || !FilePath.IsEmpty(); }
    public:
        CHash Hash;
        CFilePath FilePath;
    };
}