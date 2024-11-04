#include "mem_allocator.h"

#include "hook.h"

MH_DefineFunc(CAllocatorMM_Malloc, 0x0058922c, TOC1, void*, CAllocatorBucket& bucket, u32 size);
MH_DefineFunc(CAllocatorMM_Free, 0x00588714, TOC1, void, CAllocatorBucket& bucket, void* data);
MH_DefineFunc(CAllocatorMM_Realloc, 0x0058a448, TOC1, void*, CAllocatorBucket& bucket, void* data, u32 size);

void* CAllocatorMM::Malloc(CAllocatorBucket& bucket, u32 size)
{
	return CAllocatorMM_Malloc(bucket, size);
}

void CAllocatorMM::Free(CAllocatorBucket& bucket, void* data)
{
	CAllocatorMM_Free(bucket, data);
}

void* CAllocatorMM::Realloc(CAllocatorBucket& bucket, void* data, u32 size)
{
	return CAllocatorMM_Realloc(bucket, data, size);
}

/* mem_allocator.cpp: 757 */
// TODO: I don't fucking know, man
u32 CAllocatorMM::ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof) 
{
	u64 uVar1 = ((u64)(s32)old_max_size & 0x7fffffffU) * 2;
	if (item_sizeof != 0) {
		uVar1 = uVar1 - (-(u64)(uVar1 < 0x20 / (u64)item_sizeof) &
                    uVar1 - 0x20 / (u64)item_sizeof);
	}
	return (s32)uVar1 -
         ((s32)uVar1 - new_size &
         -(u32)((uVar1 & 0xffffffff) < ((u64)(s32)new_size & 0xffffffffU)));
}