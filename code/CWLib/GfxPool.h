#ifndef GFX_POOL_H
#define GFX_POOL_H



class CGfxMemoryPool {
    class CAllactionInfo {
    public:
        u32 StartOffset;
        u32 Size;
        u32 AlignMinus1;
        u32 NextHandle;
    };

    class CFreeInfo {
    public:
        u32 StartOffset;
        u32 EndOffset;
        CFreeInfo* Next;
    };

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

extern CGfxMemoryPool g_MemPools[];

#endif // GFX_POOL_H