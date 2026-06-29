#pragma once

#include <mem_allocator.h>

class CMemPoolSimple {
struct SBlock { SBlock* Next; };
public:
    inline CMemPoolSimple(u32 block_size) : BlockSize(block_size), FirstBlock(), CurrentPtr(), CurrentPtrEnd()
    {

    }

    inline ~CMemPoolSimple()
    {
        SBlock* block = FirstBlock;
        while (block != NULL)
        {
            SBlock* next = block->Next;
            CAllocatorMM::Free(gOtherBucket, block);
            block = next;
        }
    }
public:
    inline void* alloc(u32 size)
    {
        SBlock* block;
        if (CurrentPtr == NULL || CurrentPtrEnd < CurrentPtr + size)
        {
            block = (SBlock*)CAllocatorMM::Malloc(gOtherBucket, BlockSize);
            block->Next = FirstBlock;
            CurrentPtr = (u8*)(block + 1);
            FirstBlock = block;
            CurrentPtrEnd = (u8*)block + BlockSize;
        }

        void* data = (void*)CurrentPtr;
        CurrentPtr += size;
        return data;
    }
private:
    u32 BlockSize;
    SBlock* FirstBlock;
    u8* CurrentPtr;
    const u8* CurrentPtrEnd;
};
