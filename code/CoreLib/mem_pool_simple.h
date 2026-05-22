#pragma once

#include <mem_allocator.h>

class CMemPoolSimple {
struct SBlock { SBlock* Next; };
public:
    inline CMemPoolSimple(u32 block_size) : BlockSize(block_size), FirstBlock(), CurrentPtr(), CurrentPtrEnd()
    {

    }

    ~CMemPoolSimple();
public:
    inline void* alloc(u32 size)
    {
        SBlock* block;
        if (CurrentPtr == NULL || CurrentPtrEnd < CurrentPtr + size)
        {
            block = (SBlock*)CAllocatorMM::Malloc(gOtherBucket, BlockSize);
            block->Next = FirstBlock;
            CurrentPtr = block + 1;
            FirstBlock = block;
            CurrentPtrEnd = (u8*)block + BlockSize;
        }

        block = (SBlock*)CurrentPtr;
        block->Next = (SBlock*)(CurrentPtr + size);
        return block;
    }
private:
    u32 BlockSize;
    SBlock* FirstBlock;
    u8* CurrentPtr;
    const u8* CurrentPtrEnd;
};
