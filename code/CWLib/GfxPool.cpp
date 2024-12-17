#include "GfxPool.h"

void* CSimpleMemoryPool::Alloc(u32 size, u32 align, u32 leave_some_free)
{
    Size -= leave_some_free;
    void* data = TentativeAlloc(size, align, Mark);
    Size += leave_some_free;
    return data;
}

void* CSimpleMemoryPool::TentativeAlloc(u32 size, u32 align, u32& newmark)
{
    u32 begin = (align + Mark) - 1 & -align;
    u32 end = begin + (-align & (align + size) - 1);
    
    if (end <= Size)
    {
        newmark = end;
        return (void*)(Address + begin);
    }

    return NULL;
}