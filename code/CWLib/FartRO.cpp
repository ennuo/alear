#include <FartRO.h>
#include <algorithm>
#include <thread.h>
#include <DebugLog.h>

struct SCompareFAT
{
    bool operator()(CFartRO::CFAT const& lhs, CFartRO::CFAT const& rhs) const
    {
        return lhs < rhs;
    }

    bool operator()(CFartManyRO::CFAT const& lhs, CFartRO::CFAT const& rhs) const
    {
        return lhs < rhs;
    }

    bool operator()(CHash const& lhs, CFartRO::CFAT& rhs) const
    {
        return lhs < rhs.hash;
    }

    bool operator()(CFartRO::CFAT const& lhs, CHash const& rhs) const
    {
        return lhs.hash < rhs;
    }
};

CFartRO::CFartRO(const CFilePath& _fp, bool enabled, bool sec, bool want_check_cache) : CCache(fp.GetFilename()),
fp(_fp), FAT(), State(EFartROState_Disabled), Secure(sec), WantCheckCache(want_check_cache), Slow(false), fd(INVALID_FILE_HANDLE), Readers(0)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    if (enabled && !OpenCache())
        CloseCache(false);
}

CFartRO::~CFartRO()
{

}

bool CFartRO::TryCloseCache(bool updating)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    if (Readers != 0) return false;
    FAT.resize(0);
    State = updating ? EFartROState_Updating : EFartROState_Disabled;
    return true;
}

void CFartRO::CloseCache(bool updating)
{
    while (true)
    {
        if (TryCloseCache(updating)) break;
        ThreadSleep(1);
    }
}

MH_DefineFunc(CFartRO_OpenCache, 0x00086680, TOC0, bool, CFartRO*);
bool CFartRO::OpenCache()
{
    return CFartRO_OpenCache(this);
}

bool CFartRO::GetReader(const CHash& hash, SResourceReader& out)
{
    ETryGetReaderResult state;
    while (true)
    {
        state = TryGetReader(hash, out);
        if (state != ETryGetReaderResult_Wait) break;
        ThreadSleep(1);
    }

    return state == ETryGetReaderResult_True;
}

bool CFartRO::GetReader(CFAT* f, SResourceReader& out)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);

    if (fd == INVALID_FILE_HANDLE)
    {
        if (!FileOpen(fp, out.Handle, Secure ? OPEN_SECURE : OPEN_READ))
            return false;
    }
    else
    {
        out.Handle = fd;
        fd = INVALID_FILE_HANDLE;
    }

    Readers++;
    out.OriginalHash = f->hash;
    out.OwnerData = Slow;
    out.BytesRead = 0;
    out.RollingHash.Reset();
    out.Size = f->size;
    out.Offset = f->offset;

    FileSeek(out.Handle, f->offset, FILE_BEGIN);

    return true;
}

CFartRO::CFAT* CFartRO::Find(const CHash& hash)
{
    CFAT* f = std::lower_bound(FAT.begin(), FAT.end(), hash, SCompareFAT());
    return f != NULL && f->hash == hash ? f : NULL;
}

bool CFartRO::Unlink(const CHash& hash)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    CFAT* f = Find(hash);
    if (f != NULL)
    {
        FAT.erase(f);
        return true;
    }

    return false;
}

bool CFartRO::GetSize(const CHash& hash, u32& out)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    CFAT* f = Find(hash);

    if (f == NULL) return false;

    out = f->size;
    return true;
}

bool CFartRO::IsSlow(const SResourceReader& rdr)
{
    return (bool)rdr.OwnerData;
}

ETryGetReaderResult CFartRO::TryGetReader(const CHash& hash, SResourceReader& out)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    ETryGetReaderResult result = ETryGetReaderResult_Wait;
    if (State != EFartROState_Updating)
    {
        CFAT* f = Find(hash);
        if (f != NULL && GetReader(f, out))
            result = ETryGetReaderResult_True;
        else
            result = ETryGetReaderResult_False;
    }

    return result;
}

void CFartRO::CloseReader(SResourceReader& in, bool hashes_matched)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);

    if (fd == INVALID_FILE_HANDLE)
    {
        fd = in.Handle;
        in.Handle = INVALID_FILE_HANDLE;
    }
    else FileClose(in.Handle);
    
    if (Readers != 0) Readers--;
    if (!hashes_matched) Unlink(in.OriginalHash);
}

bool CFartRO::Put(CHash& hash_in_out, const void* bin, u32 size)
{
    return false;
}

MH_DefineFunc(MakeROCache, 0x000869ac, TOC0, CFartRO*, const CFilePath& farc_file, bool enabled, bool secure);
