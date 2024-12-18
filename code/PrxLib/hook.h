#ifndef HOOK_H
#define HOOK_H

#include "ppcasm.h"

#define TOC0 (0x009230b8)
#define TOC1 (0x00932B50)

#define PORT_BIND_TEXTURE (0x001cadc8)

struct v4hack { v4 V; };
extern void* gTocBase;
extern u8* gStubBaseAddress;

struct shkOpd
{
    void* Function;
    void* TOC;
};

struct ps3_write {

    ps3_write() {}
    ps3_write(u32 addr, u32 word) { Address = addr; Word = word; }
    
    u32 Address;
    u32 Word;
};

#define MAX_WRITES (1024)
extern ps3_write gWriteCache[MAX_WRITES];
extern u32 gNumWrites;

u32 sys_dbg_read_process_memory(uint64_t address, void* data, size_t size);
u32 sys_dbg_write_process_memory(uint64_t address, void* data, size_t size);

void* MH_Allocate(size_t size);
void* MH_Shellcode(u32* data, size_t size);

inline void MH_Poke32(u32 address, u32 value)
{
    sys_dbg_write_process_memory((u64)address, (void*)&value, sizeof(u32));
}

inline u32 MH_Read32(u32 address)
{
    u32 value;
    sys_dbg_read_process_memory((u64)address, (void*)&value, sizeof(u32));
    return value;
}

#define MH_Write(address, data, size) sys_dbg_write_process_memory((u64)(address), (void*)(data), (size));
#define MH_Read(address, data, size) sys_dbg_read_process_memory((u64)(address), (void*)(data), (size));

// void MH_InitHook(void* address, void* hook, void* original_function);
void MH_InitHook(void* address, void* hook);

#define MH_DefineFunc(name, address, toc, type, ...) \
static shkOpd _##name = { (void*)(address), (void*)(toc) }; \
type(*name)(__VA_ARGS__) = (type (*)(__VA_ARGS__))&_##name;

#define MH_PokeBranch(address, target) MH_Poke32(address, B(target, address))

#endif // HOOK_H