#pragma once

#include <Sync/Types.h>

namespace sync
{
    enum
    {
        kResponse_Ok = 0,
        kResponse_Fail = 0x8000,
        kResponse_BadRequest,
        kResponse_Unauthorized,
        kResponse_Forbidden,
        kResponse_NotFound,
        kResponse_MethodNotAllowed
    };

    struct msg_download
    {
        token Token;
        CGUID GUID;
        CHash Hash;
    };

    struct msg_upload
    {
        token Token;
        CHash Hash;
        u32 FileSize;
    };
    
    struct msg_download_response
    {
        CHash FileHash;
        u32 FileSize;
        bool IsCompressed;
        u32 CompressedFileSize;
    };

    struct msg_login
    {
        msg_login() : Method(), Ticket(), Username(), Password()
        {
        }
        
        int Method;
        ByteArray Ticket;
        MMString<char> Username;
        MMString<char> Password;
    };

    struct msg_login_response
    {
        u64 AccountID;
        token Token;
    };

    struct msg_depot_response
    {
        u64 DepotID;
        u64 CommitID;
        ByteArray FileData;
    };
    
    struct msg_depot_list
    {
        u64 Permissions;
        CVector<depot> Depots;
    };

    struct msg_depot
    {
        u64 DepotID;
        struct
        {
            MMString<char> Name;
        } Edit;
    };

    struct msg_planet_list
    {
        inline msg_planet_list() : Planets()
        {

        }
        
        CVector<SSerializedCrafteroid> Planets;
    };

    struct msg_resource_list
    {
        inline msg_resource_list() : Hashes()
        {

        }
        
        CRawVector<CHash> Hashes;
    };

    template <typename T> inline u32 GetMessageType();
    template <typename T> inline u32 GetChannel();

    #define REGISTER(type, channel, message) \
        template <> inline u32 GetMessageType<type>() { return message; } \
        template <> inline u32 GetChannel<type>() { return channel; }

    REGISTER(msg_download, eChannel_Resource, eMessageType_Download);
    REGISTER(msg_download_response, eChannel_Resource, eMessageType_Download);
    REGISTER(msg_login_response, eChannel_Gate, eMessageType_Login);
    REGISTER(msg_login, eChannel_Gate, eMessageType_Login);
    REGISTER(msg_resource_list, eChannel_Resource, eMessageType_FilterResources);
    
    #undef REGISTER
}