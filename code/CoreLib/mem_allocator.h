#ifndef MEM_ALLOCATOR_H
#define MEM_ALLOCATOR_H

#include "mmtypes.h"
#include "mem.h"

namespace MM {
    enum AllocType {
        UNKNOWN = 0x0,
        STL = 0x1,
        VECTOR = 0x2,
        CONTAINER = 0x3,
        GFX = 0x4,
        BINK = 0x5,
        RESOURCE = 0x6,
        PROFILE = 0x7,
        NUM_ALLOC_TYPES = 0x8
    };
}

/* mem_allocator.h: 36 */
class CAllocatorBucket {
public:
    u64 Timer[6];
    void* Allocator;
    u8 Type;
};

extern CAllocatorBucket g_OtherBucket;
extern CAllocatorBucket g_VectorBucket;

/* mem_allocator.h: 85 */
class CAllocatorMM {
public:
    static void* Malloc(CAllocatorBucket& bucket, u32 size);
    static void Free(CAllocatorBucket& bucket, void* data);
    static void* Realloc(CAllocatorBucket& bucket, void* data, u32 size);
    static u32 ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof);
};


#endif // MEM_ALLOCATOR_H