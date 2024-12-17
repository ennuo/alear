#ifndef GFX_POOL_H
#define GFX_POOL_H

class CSimpleMemoryPool {
public:
    inline u32 GetOffset(void* addy)
    {
        return ((u32)addy - Address) + Offset;
    }
    
    void* Alloc(u32 size, u32 align, u32 leave_some_free);
private:
    void* TentativeAlloc(u32 size, u32 align, u32& new_mark);
private:
    u32 Address;
    u32 Offset;
    u32 Size;
    u32 Mark;
    u32 LastMark;
    u32 HeapMonID;
};

class CGfxMemoryPool {
    class CAllactionInfo {
    public:
        u32 StartOffset;
        u32 Size;
        u32 AlignMinus1;
        u32 NextHandle;
    public:
        inline u32 GetAlignedOffset() const
        {
            return (StartOffset + AlignMinus1) & (~AlignMinus1);
        }
    };

    class CFreeInfo {
    public:
        u32 StartOffset;
        u32 EndOffset;
        CFreeInfo* Next;
    public:
        inline u32 Size() { return EndOffset - StartOffset; }
        inline bool CouldFit(u32 size, u32 align)
        {
            return size + (StartOffset + (align - 1) & ~(align - 1)) <= EndOffset;
        }
    };

public:
    inline void* GetAddress(u32 handle) const
    {
        return (void*)(Address + Handles[handle].GetAlignedOffset());
    }

    inline u32 GetOffset(u32 handle) const
    {
        return Offset + Handles[handle].GetAlignedOffset();
    }
private:
    CFreeInfo* FreeInfos;
    CAllactionInfo* Handles;
    CFreeInfo* FirstFree;
    CFreeInfo* FirstFreeInfo;
    u32 FirstFreeHandle;
    u32 Address;
    u32 Offset;
    u32 Size;
    u32 Location;
    u32 MaxHandles;
    u32 Align;
    u32 SizeUsed;
    u32 PrevSizeUsed;
    u32 HeapMonID;
};

class CGfxHandle {
public:
    inline CGfxHandle()
    {
        Handle = -1;
        Pool = NULL;
    }
public:
    inline CGfxMemoryPool* GetPool() const { return Pool; }
    inline void* GetCachedAddress() const { return CachedAddress; }
    inline void* GetAddress() const { return Pool->GetAddress(Handle); }
    inline u32 GetOffset() const { return Pool->GetOffset(Handle); }
private:
    void* CachedAddress;
    u32 Handle;
    CGfxMemoryPool* Pool;
};

enum EMemPool 
{
    MEM_POOL_LOCAL,
    MEM_POOL_MAIN_MAPPED,
    MEM_POOL_MAIN_UNMAPPED,
    MEM_POOL_LOCAL_FRAMEBUF,
    MEM_POOL_LOCAL_ZBUF,
    MEM_POOL_LOCAL_RENDERTAGETS,
    MEM_POOL_MAIN_RENDERTAGETS,

    MEM_POOL_LAST
};

enum ESimpleMemPool
{
    SMEM_POOL_MAIN_MAPPED_DYNAMIC_GEOM_POS0,
    SMEM_POOL_MAIN_MAPPED_DYNAMIC_GEOM_POS1,
    SMEM_POOL_MAIN_MAPPED_DYNAMIC_GEOM_POS2,
    SMEM_POOL_MAIN_MAPPED_DYNAMIC_GEOM_NOR,
    SMEM_POOL_MAIN_MAPPED_DYNAMIC_GEOM_NOR1,

    SMEM_POOL_LAST
};

extern CGfxMemoryPool gMemPools[];
extern CSimpleMemoryPool gSMemPools[];

#endif // GFX_POOL_H