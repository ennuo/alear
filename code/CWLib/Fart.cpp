#include <FartRO.h>
#include <GuidHashMap.h>
#include <ResourceSystem.h>
#include <DebugLog.h>

CCache::CCache(const char* name) : Mutex(name)
{
    
}

CCache::~CCache()
{
    
}

MH_DefineFunc(InitCaches, 0x000814f0, TOC0, bool);
MH_DefineFunc(CloseCaches, 0x0007f2f0, TOC0, bool);

MH_DefineFunc(IncorporateFartRO, 0x0008026c, TOC0, bool, CFartRO* cache);

MH_DefineFunc(GetFileDataFromCaches, 0x000816bc, TOC0, bool, CHash&, ByteArray&);
MH_DefineFunc(SaveFileDataToCache, 0x0007f458, TOC0, bool, CacheType type, const void* data, u32 size, CHash& out);

bool GetResourceReader(CHash& hash, SResourceReader& out)
{
    for (int i = 0; i < CT_COUNT; ++i)
    {
        CCache* cache = gCaches[i];
        if (cache != NULL && cache->GetReader(hash, out))
            return true;
    }

    return false;
}

bool GetResourceReader(const CResourceDescriptorBase& desc, SResourceReader& out, CFilePath& loose_path)
{
    CHash hash = desc.LatestHash();
    ESerialisationType st = GetPreferredSerialisationType(desc.GetType());

    if (hash && GetResourceReader(hash, out))
        return true;
    
    const CFileDBRow* row;
    if (desc.HasGUID()) row = FileDB::FindByGUID(desc.GetGUID());
    // else row = FileDB::FindByHash(desc.GetHash());

    if (row == NULL) return false;

    CFilePath paths[] =
    {
        CFilePath(FPR_GAMEDATA, row->GetPath()),
        CFilePath(FPR_BLURAY, row->GetPath())
    };

    for (u32 i = 0; i < ARRAY_LENGTH(paths); ++i)
    {
        CFilePath& fp = paths[i];
        if (fp.IsEmpty() || !FileExists(fp)) continue;

        if (st == PREFER_FILE)
        {
            loose_path = fp;
            return true;
        }

        if (i == FPR_GAMEDATA)
        {
            if (!FileOpen(fp, out.Handle, OPEN_READ)) return false;
            out.Size = FileSize(out.Handle);
            out.Offset = 0;
            out.OriginalHash = hash;

            return true;
        }

        loose_path = fp;
        return false;
    }
    
    return false;
}

bool FileClose(SResourceReader& h)
{
    if (h.Handle == INVALID_FILE_HANDLE) return true;

    bool hashes_match = true;

    if (h.BytesRead > 0 && h.BytesRead == h.Size && h.OriginalHash)
    {
        CHash hash;
        if (h.RollingHash.Result((uint8_t*)&hash) == SHA1_SUCCESS && hash != h.OriginalHash && h.Owner != NULL)
        {
            MMLogCh(DC_PLAYER_PROFILE, "Hash mismatch: original: %s, rolling: %s, handle: %d\n",
                StringifyHash(h.OriginalHash).c_str(),
                StringifyHash(hash).c_str(),
                (int)h.Handle
            );

            hashes_match = false;
        }
    }

    if (h.Owner != NULL) h.Owner->CloseReader(h, true);
    FileClose(h.Handle);
    h.Handle = INVALID_FILE_HANDLE;

    return hashes_match;
}

u64 FileSize(SResourceReader& h)
{
    return h.Size;
}

u64 FileRead(SResourceReader& h, void* out, u64 count)
{
    u64 n = FileRead(h.Handle, out, count);
    if (h.BytesRead != -1)
    {
        h.Offset += n;
        h.BytesRead += n;
        h.RollingHash.AddData((const uint8_t*)out, count);
    }

    return n;
}

bool FileLoad(SResourceReader& h, ByteArray& out)
{
    u64 count = FileSize(h);
    u64 n = 0;

    if (out.try_resize(count))
        n = FileRead(h, out.begin(), count);

    return FileClose(h) && n == count;
}