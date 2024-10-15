#include "mmtypes.h"
#include "mem_allocator.h"

void* operator new(size_t n)
{
    return CAllocatorMM::Malloc(g_OtherBucket, n);
}

void* operator new[](size_t sz)
{
    return CAllocatorMM::Malloc(g_OtherBucket, sz);
}

void operator delete(void* p)
{
    CAllocatorMM::Free(g_OtherBucket, p);
}

void operator delete[](void* p)
{
    CAllocatorMM::Free(g_OtherBucket, p);
}

extern "C" void* ps3_malloc(size_t size)
{
    return CAllocatorMM::Malloc(g_OtherBucket, size);
}

extern "C" void ps3_free(void* p)
{
    CAllocatorMM::Free(g_OtherBucket, p);
}

extern "C" void* ps3_realloc(void* ptr, size_t new_size)
{
    return CAllocatorMM::Realloc(g_OtherBucket, ptr, new_size);
}
