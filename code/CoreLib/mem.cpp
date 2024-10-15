
#include "mem_allocator.h"

void* operator new(size_t n)
{
    return CAllocatorMM::Malloc(gOtherBucket, n);
}

void* operator new[](size_t sz)
{
    return CAllocatorMM::Malloc(gOtherBucket, sz);
}

void operator delete(void* p)
{
    CAllocatorMM::Free(gOtherBucket, p);
}

void operator delete[](void* p)
{
    CAllocatorMM::Free(gOtherBucket, p);
}

extern "C" void* ps3_malloc(size_t size)
{
    return CAllocatorMM::Malloc(gOtherBucket, size);
}

extern "C" void ps3_free(void* p)
{
    CAllocatorMM::Free(gOtherBucket, p);
}

extern "C" void* ps3_realloc(void* ptr, size_t new_size)
{
    return CAllocatorMM::Realloc(gOtherBucket, ptr, new_size);
}
