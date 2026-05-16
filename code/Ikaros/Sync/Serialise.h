#pragma once

#include <SharedSerialise.h>
#include <Sync/Types.h>
#include <Sync/Messages.h>
#include <filepath.h>

using namespace sync;

enum SyncSR
{
    eSyncSR_Initial = 1,
    eSyncSR_LatestPlusOne
};

const u32 kProtocolVersion = eSyncSR_LatestPlusOne - 1;

class CSyncSaveVector : public CReflectionSaveVector {
public:
    inline CSyncSaveVector() : Data(), CReflectionSaveVector(&Data, 0) 
    {
        SetCompressionFlags(0);
    }
public:
    inline u32 GetRevision() { return eSyncSR_LatestPlusOne - 1; }
    inline u32 GetBranchDescription() { return 0; }
    inline u16 GetBranchID() { return 0; }
    inline u16 GetBranchRevision() { return 0; }
public:
    ByteArray Data;
};

template <typename R>
ReflectReturn Reflect(R& r, NetworkOnlineID& d)
{
    return r.ReadWrite((void*)&d, sizeof(NetworkOnlineID));
}

template <typename R>
ReflectReturn Reflect(R& r, token& d)
{
    return r.ReadWrite((void*)&d, sizeof(token));
}

template <typename R>
ReflectReturn Reflect(R& r, CFilePath& d)
{
    ReflectReturn rv = REFLECT_OK;
    s32 length = d.Length();
    if ((rv = Reflect(r, length)) != REFLECT_OK) return rv;

    if (r.GetSaving())
        return r.ReadWrite((void*)d.c_str(), length);

    if (r.IsGatherVariables()) return rv;
    
    char filename[MAX_PATH];
    if (length >= MAX_PATH) 
        return REFLECT_EXCESSIVE_DATA;

    if ((rv = r.ReadWrite((void*)filename, length)) != REFLECT_OK)
        return rv;

    d.Assign(filename);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, extension_info& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(ProtocolVersion);
    ADD(MessageChannel);
    ADD(Id);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, SServerInfo& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(ProtocolVersion);
    ADD(Name);
    ADD(Version);
    ADD(BuildDate);
    ADD(Extensions);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, depot& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(DepotID);
    ADD(CommitID);

    int len = StringLength(d.Id);
    if ((rv = Reflect(r, len)) != REFLECT_OK) return rv;
    if ((rv = r.ReadWrite(d.Id, len)) != REFLECT_OK) return rv;
    if (r.GetLoading()) d.Id[len] = '\0';

    len = StringLength(d.Name);
    if ((rv = Reflect(r, len)) != REFLECT_OK) return rv;
    if ((rv = r.ReadWrite(d.Name, len)) != REFLECT_OK) return rv;
    if (r.GetLoading()) d.Name[len] = '\0';
    
    ADD(Priority);
    ADD(Flags);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, msg_depot_response& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(DepotID);
    ADD(CommitID);
    ADD(FileData);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, SSerializedCrafteroid& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(RemoteId);
    ADD(OwnerId);
    ADD(Name);
    ADD(PlanetDecorations);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, msg_planet_list& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(Planets);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, msg_resource_list& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(Hashes);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, msg_download_response& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(FileHash);
    ADD(FileSize);

    ADD(IsCompressed);
    if (d.IsCompressed)
    {
        ADD(CompressedFileSize);
    }
    else if (r.GetLoading())
        d.CompressedFileSize = d.FileSize;
    
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, msg_login& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(Method);
    if (d.Method == eAuthenticationType_Ticket)
    {
        ADD(Ticket);
    }
    else if (d.Method == eAuthenticationType_Credentials)
    {
        ADD(Username);
        ADD(Password);
    }
    
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, msg_login_response& d)
{
    ReflectReturn rv = REFLECT_OK;
    ADD(AccountID);
    ADD(Token);
    return rv;
}

template <typename R>
ReflectReturn Reflect(R& r, msg_download& d)
{
    ReflectReturn rv = REFLECT_OK;

    if (r.GetLoading())
        memset(&d, 0, sizeof(msg_download));
    
    if ((rv = Reflect(r, d.Token)) != REFLECT_OK)
        return rv;

    const u8 HASH_BITFIELD = 1;
    const u8 GUID_BITFIELD = 2;
    
    u8 guid_hash_inline_bitfield = 0;
    if (r.GetSaving())
    {
        if (d.GUID) guid_hash_inline_bitfield |= GUID_BITFIELD;
        if (d.Hash) guid_hash_inline_bitfield |= HASH_BITFIELD;
    }

    if ((rv = Reflect(r, guid_hash_inline_bitfield)) != REFLECT_OK) return rv;

    ADD_CONDITIONAL(eSyncSR_Initial, guid_hash_inline_bitfield & GUID_BITFIELD, GUID);
    ADD_CONDITIONAL(eSyncSR_Initial, guid_hash_inline_bitfield & HASH_BITFIELD, Hash);

    return rv;
}