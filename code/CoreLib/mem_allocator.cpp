#include "mem_allocator.h"

#include "hook.h"

MH_DefineFunc(CAllocatorBucket_AlignedRealloc, 0x00589934, TOC1, void*, CAllocatorBucket& bucket, void* data, u32 size, u32 align);
MH_DefineFunc(CAllocatorBucket_AlignedMalloc, 0x00588a60, TOC1, void*, CAllocatorBucket& bucket, u32 size, u32 align);
MH_DefineFunc(CAllocatorBucket_AlignedFree, 0x005888b8, TOC1, void, CAllocatorBucket& bucket, void* data);
MH_DefineFunc(CAllocatorBucket_Malloc, 0x0058922c, TOC1, void*, CAllocatorBucket& bucket, u32 size);
MH_DefineFunc(CAllocatorBucket_Free, 0x00588714, TOC1, void, CAllocatorBucket& bucket, void* data);
MH_DefineFunc(CAllocatorBucket_Realloc, 0x0058a448, TOC1, void*, CAllocatorBucket& bucket, void* data, u32 size);

void* CAllocatorMM::Malloc(CAllocatorBucket& bucket, u32 size)
{
	return CAllocatorBucket_Malloc(bucket, size);
}

void CAllocatorMM::Free(CAllocatorBucket& bucket, void* data)
{
	CAllocatorBucket_Free(bucket, data);
}

void* CAllocatorMM::Realloc(CAllocatorBucket& bucket, void* data, u32 size)
{
	return CAllocatorBucket_Realloc(bucket, data, size);
}

/* mem_allocator.cpp: 757 */
// TODO: I don't fucking know, man
u32 CAllocatorMM::ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof) 
{
	u32 double_size = old_max_size * 2;
	u32 min_size = double_size;

	if (item_sizeof != 0)
	{
		min_size = 32 / item_sizeof;
		if (min_size < double_size)
			min_size = double_size;
	}

	return new_size > min_size ? new_size : min_size;
}

void* CAllocatorMMAligned128::Malloc(CAllocatorBucket& bucket, u32 size)
{
	return CAllocatorBucket_AlignedMalloc(bucket, size + 0x7f & 0xffffff80, 128);
}

void CAllocatorMMAligned128::Free(CAllocatorBucket& bucket, void* data)
{
	CAllocatorBucket_AlignedFree(bucket, data);
}

void* CAllocatorMMAligned128::Realloc(CAllocatorBucket& bucket, void* data, u32 size)
{
	return CAllocatorBucket_AlignedRealloc(bucket, data, size + 0x7f & 0xffffff80, 128);
}

u32 CAllocatorMMAligned128::ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof) 
{
	u32 double_size = old_max_size * 2;
	u32 min_size = double_size;

	if (item_sizeof != 0)
	{
		min_size = 32 / item_sizeof;
		if (min_size < double_size)
			min_size = double_size;
	}

	return new_size > min_size ? new_size : min_size;
}