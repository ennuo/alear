#include "hook.h"

#include <sys/process.h>
#include <sys/syscall.h>

#include "cell/DebugLog.h"


#include "ppcasm.h"

void* gTocBase;

void* GetTOCBase()
{
    asm("mr %r3, %r2");
}

u32 sys_dbg_read_process_memory(uint64_t address, void* data, size_t size)
{
    system_call_4(904, (u64)sys_process_getpid(), address, size, (u64)data);
    return_to_user_prog(u32);
}

u32 sys_dbg_write_process_memory(uint64_t address, void* data, size_t size)
{
    system_call_4(905, (u64)sys_process_getpid(), address, size, (u64)data);
    return_to_user_prog(u32);
}

static u8 g_StubData[0x10000] __attribute__((section(".text#"))) = { 0 };
static u8* g_StubAddress = g_StubData;
static bool g_HookInit = false;

void* MH_Allocate(size_t size)
{
    void* data = (void*)g_StubAddress;
    g_StubAddress += size;
    return data;
}

void* MH_Shellcode(u32* data, size_t size)
{
    void* stub = MH_Allocate(size);
    MH_Write(stub, data, size);
    return stub;
}

void MH_InitHook(void* address, void* hook)
{
    if (!g_HookInit)
    {
        gTocBase = GetTOCBase();
        g_HookInit = true;
    }

    u32 hook_fn_data = ((u32*)hook)[0];
    u32 hook_fn_toc = ((u32*)hook)[1];

    // This is excessive on the codesize,
    // but I don't really care too much honestly.
    u32 shellcode[] =
    {
        // Function prologue
        0xf821ff01, // stdu %r1, -0x100(%r1)
        0x7c0802a6, // mflr %r0
        0xf8010110, // std %r0, 0x110(%r1)
        0xf8410028, // std %r2, 0x28(%r1)

        // Fixup our TOC base and push the function address
        LIS(2, (hook_fn_toc >> 16)),
        ORI(2, 2, (hook_fn_toc & 0xffff)),
        LIS(0, (hook_fn_data >> 16)),
        ORI(0, 0, (hook_fn_data & 0xffff)),

        // Call the hook function
        0x7c0903a6, // mtctr %r0
        0x4e800421, // bctrl

        // Epilogue
        0xe8410028, // ld r2, 0x28(r1)
        0xe8010110, // ld r0, 0x110(r1)
        0x7c0803a6, // mtlr r0
        0x38210100, // addi r1, r1, 0x100
        0x4e800020 // blr
    };


    
    void* stub_fn = MH_Shellcode(shellcode, sizeof(shellcode));
    u32 branch = B(stub_fn, address);
    MH_Write(address, &branch, sizeof(u32));
}

void MH_InitHook(void* address, void* hook, void* original_function)
{
    u32 original = MH_Read32((u32)address);
    MH_InitHook(address, hook);
    if (original_function != NULL)
    {
        
    }
}