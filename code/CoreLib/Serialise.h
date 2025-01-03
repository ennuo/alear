#ifndef SERIALISE_H
#define SERIALISE_H

#include "SerialiseEnums.h"
#include "MMString.h"
#include "AlearSR.h"
#include "vector.h"
#include "GuidHash.h"
#include "fifo.h"

const u8 COMPRESS_INTS = 0x1; // file.h: 10
const u8 COMPRESS_VECTORS = 0x2; // file.h: 11
const u8 COMPRESS_MATRICES = 0x4; // file.h: 12

const u8 DEFAULT_COMPRESS_FLAGS = 0x7; // file.h: 18

class PWorld;

struct SRevision {
    u32 Revision;
    u32 BranchDescription;
};

class CReflectionBase { // file.h : 91
public:
    inline CReflectionBase() : 
    NumVisited(), CanVisitThings(), World(), LazyCPPriority(),
    ThingPtrAsUID(), TakenReflectionCS(), TempString(), DependencyCollector()
    {

    }
    
    virtual ~CReflectionBase() {}; // force vtable gen
public:
    inline bool IsGatherVariables() { return false; }
    inline bool GetCompressInts() { return false; }
    inline bool GetCompressVectors() { return false; }
    inline bool GetCompressMatrices() { return false; }
private:
    u32 NumVisited;
    CRawVector<void*> CanVisitThings;
    PWorld* World;
protected:
    CStreamPriority LazyCPPriority;
    bool ThingPtrAsUID;
    bool TakenReflectionCS;
    MMString<char> TempString;
private:
    void* DependencyCollector; // CDependencyCollector*
};

class CReflectionVisitLoad {
private:
    CRawVector<void*> Visited;
};

class CReflectionVisitSave {

};

class CReflectionLoadVector : public CReflectionBase, public CReflectionVisitLoad { // file.h: 181
public:
    CReflectionLoadVector(CBaseVector<char>* vec);
public:
    ReflectReturn ReadWrite(void* d, int size);
    ReflectReturn LoadCompressionData(u32* totalsize);
    ReflectReturn CleanupDecompression();
public:
    inline u16 GetCustomVersion() { return AlearResourceRevision; }
    inline bool GetCompressInts() { return (CompressionFlags & COMPRESS_INTS) != 0; }
    inline u8 GetCompressionFlags() { return CompressionFlags; }
    inline void SetCompressionFlags(u8 flags) { CompressionFlags = flags; }
    inline bool GetSaving() { return false; }
    inline bool GetLoading() { return true; }
    inline bool IsGatherVariables() { return false; }
    inline u32 GetRevision() { return Revision.Revision; }
    inline u32 GetVecLeft() { return Vec->size() - LoadPos; }
    inline bool RequestToAllocate(u64 size)
    {
        Allocated += size;
        return Allocated < 9999999; // ~10mbs
    }

    inline ReflectReturn StartCompressing() { return REFLECT_NOT_IMPLEMENTED; }
    inline ReflectReturn FinishCompressing() { return REFLECT_NOT_IMPLEMENTED; }

    inline void Align(int a)
    {
        LoadPos = (a + LoadPos) -1 & -a;
    }

protected:
    CBaseVector<char>* Vec;
    u16 AlearResourceRevision; // padding lets me just slip this in, gets pulled from the CompressionVersion field
    u64 Allocated;
    CRawVector<char> DecryptedVec;
    SRevision Revision;
    u32 LoadPos;
    u32 OutstandingBuffers;
    void* CurJob; // these two fields are CP<DecompressionJob> i just dont really care
    void* PendingInit;
private:
    CMMSemaphore* Sem;
    u32 JobsTag;
    bool SPUDecompressAvailable;
    u8 CompressionFlags;
};

class CReflectionSaveVector : public CReflectionBase, public CReflectionVisitSave { // file.h: 259
public:
    ReflectReturn ReadWrite(void* d, int size);
    ReflectReturn StartCompressing();
    ReflectReturn FinishCompressing();
public:
    inline u16 GetCustomVersion() { return ALEAR_LATEST_PLUS_ONE - 1; }
    inline bool GetCompressInts() { return (CompressionFlags & COMPRESS_INTS) != 0; }
    inline u8 GetCompressionFlags() { return CompressionFlags; }
    inline void SetCompressionFlags(u8 flags) { CompressionFlags = flags; }
    inline bool GetSaving() { return true; }
    inline bool GetLoading() { return false; }
    inline bool IsGatherVariables() { return false; }
    inline u32 GetRevision() { return 0x272; }
    inline bool RequestToAllocate(u64 size) { return true; }
    inline ReflectReturn CleanupDecompression() { return REFLECT_NOT_IMPLEMENTED; }
    inline ReflectReturn LoadCompressionData(u32* totalsize) { return REFLECT_NOT_IMPLEMENTED; }
    inline u32 GetVecLeft() { return 0; }
    
    inline void Align(int a)
    {
        u32 offset = Vec->size();
        Vec->try_resize((a + Vec->size()) -1 & -a);
        memset(Vec->begin() + offset, 0, Vec->size() - offset);
    }

private:
    ByteArray* Vec;
    void* CurJob;
    void* PendingCleanup;
    CMMSemaphore Sem;
    u32 EncryptedBlockStart;
    u32 JobsTag;
    u32 CompressionLevel;
    bool SPUCompressAvailable;
    u8 CompressionFlags;
};

#include "ReflectionFindDependencies.h"

template <typename R, typename D>
ReflectReturn ReflectCompressedInt(R& r, D& d)
{
    ReflectReturn ret = REFLECT_OK;

    if (r.GetLoading())
    {
        d = 0;
        int shift = 0;
        do 
        {
            u8 b;
            ret = r.ReadWrite((void*)&b, sizeof(u8));
            d |= (b & 0x7f) << (shift & 0x3f);
            if ((b & 0x80) == 0) return ret;
            shift += 7;
        } 
        while (ret == REFLECT_OK);
    }

    if (r.GetSaving())
    {
        D value = d;
        do
        {
            u8 b = value & 0x7f;
            value >>= 7;
            if (value != 0) b |= 0x80;
            ret = r.ReadWrite((void*)&b, sizeof(u8));
            if (value == 0) return ret;
        }
        while (ret == REFLECT_OK);
    }
    
    return ret;
}

template <typename R>
ReflectReturn Reflect(R& r, s8& h)
{
    return r.ReadWrite((void*)&h, sizeof(s8));
}

template <typename R>
ReflectReturn Reflect(R& r, u8& h)
{
    return r.ReadWrite((void*)&h, sizeof(u8));
}

template <typename R>
ReflectReturn Reflect(R& r, s16& h)
{
    return r.ReadWrite((void*)&h, sizeof(s16));
}

template <typename R>
ReflectReturn Reflect(R& r, u16& h)
{
    return r.ReadWrite((void*)&h, sizeof(u16));
}

template <typename R>
ReflectReturn Reflect(R& r, u32& h)
{
    if (r.GetCompressInts()) return ReflectCompressedInt(r, h);
    else return r.ReadWrite((void*)&h, sizeof(u32));
}

template <typename R>
ReflectReturn Reflect(R& r, s32& h)
{
    if (r.GetCompressInts()) 
    {
        h = (u32)(h << 1) ^ (h >> 31);
        ReflectReturn res = ReflectCompressedInt(r, h);
        h = (s32)(h >> 1) ^ (u32)(-(s32)(h & 1));
    }
    else return r.ReadWrite((void*)&h, sizeof(u32));
}

template <typename R>
ReflectReturn Reflect(R& r, float& f)
{
    return r.ReadWrite((void*)&f, sizeof(float));
}

template <typename R>
ReflectReturn Reflect(R& r, bool& b) // file.h: 371
{
    return r.ReadWrite((void*)&b, sizeof(bool));
}

template <typename R>
ReflectReturn Reflect(R& r, v4& v) // file.h: 1254
{
    return r.ReadWrite((void*)&v, sizeof(v4));
}

template <typename R>
ReflectReturn Reflect(R& r, MMString<char>& d)
{
    ReflectReturn res;
    s32 size = d.size();
    res = Reflect(r, size);
    if (res != REFLECT_OK) return res;

    // game technically calls begin which calls operator[0], but its the same as c_str
    d.resize(size, '\0');
    return r.ReadWrite((void*)d.begin(), size * sizeof(char));
}

template <typename D>
ReflectReturn Add(CReflectionLoadVector& r, D& d, char* c)
{
    return Reflect(r, d);
}

template <typename D>
ReflectReturn Add(CReflectionSaveVector& r, D& d, char* c)
{
    return Reflect(r, d);
}

template <typename D>
ReflectReturn Add(CReflectionFindDependencies& r, D& d, char* c)
{
    return Reflect(r, d);
}

template <typename R>
ReflectReturn Reflect(R& r, CGUID& d)
{
    return Reflect(r, d.guid);
    // return Add(r, d.guid, "guid");
}

#include <refcount.h>
#include <ResourceDescriptor.h>

template<typename R, typename D>
ReflectReturn Reflect(R& r, CP<D>& d);

template<typename R, typename D>
ReflectReturn Reflect(R& r, CResourceDescriptor<D>& d);


// this one is pretty simple to replicate im just still lazy tbh
template <typename R>
ReflectReturn ReflectDescriptor(R& r, CResourceDescriptorBase& d, bool cp, bool type);

class CReflectionLoadVector;
class CReflectionSaveVector;

template <>
ReflectReturn ReflectDescriptor(CReflectionLoadVector& r, CResourceDescriptorBase& d, bool cp, bool type);

template <>
ReflectReturn ReflectDescriptor(CReflectionSaveVector& r, CResourceDescriptorBase& d, bool cp, bool type);

#endif // SERIALISE_H