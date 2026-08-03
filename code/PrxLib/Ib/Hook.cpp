#include <Ib/Hook.h>
#include <Ib/Memory.h>
#include <Ib/Printf.h>
#include <Ib/Assembly/PowerPC.h>
#include <new>

#include <sys/process.h>
#include <sys/syscall.h>
#include <sys/prx.h>

using namespace Ib;

static uint64_t gNextHookIndex = 1;
StaticDetourHandle* gStaticDetourHead;

VecAllBase<DetourData>* gDetourData;
VecAllBase<char>* gBranchPool;

namespace Ib
{
    void* Allocate(size_t size)
    {
        return gBranchPool->Allocate(size);
    }

    void* Shellcode(void* data, size_t size)
    {
        void* stub = Allocate(size);
        Ib_Write(stub, data, size);
        return stub;
    }

    HookHandle ReplaceCall(uintptr_t address, const void* hook)
    {
        uint32_t hook_fn_data = ((uint32_t*)hook)[0];
        uint32_t hook_fn_toc = ((uint32_t*)hook)[1];


        const uint32_t FUNCTION_CALL_INDEX = 3;
        const uint32_t BRANCH_INDEX = 5;

        uint32_t shellcode[] =
        {
            // Store the current TOC base
            0xf8410028, // std %r2, 0x28(%r1)

            // Fixup our TOC base and push the function address
            LIS(2, (hook_fn_toc >> 16)),
            ORI(2, 2, (hook_fn_toc & 0xffff)),

            0xDEADBEEF, // Function call address to be replaced

            // Restoure our TOC base and branch back
            0xe8410028, // ld r2, 0x28(r1)
            0xDEADBEEF
        };

        void* stub = Allocate(sizeof(shellcode));
        if (stub == NULL)
            return INVALID_HOOK_HANDLE;
        
        shellcode[FUNCTION_CALL_INDEX] = BL(hook_fn_data, (uint32_t*)stub + FUNCTION_CALL_INDEX);
        shellcode[BRANCH_INDEX] = B((uint32_t)address + 4, (uint32_t*)stub + BRANCH_INDEX);
        
        Ib_Write(stub, shellcode, sizeof(shellcode));

        uint32_t branch = B(stub, address);
        Ib_Write(address, &branch, sizeof(uint32_t));

        return gNextHookIndex++;
    }

    HookHandle Hook(DetourHandle* handle)
    {
        // Assume ununitialized handle still has the original address of the function.
        uint32_t address = ((uint32_t*)handle->Original)[0];

        printf("ib: hooking detour \"%s\" @ %08x\n", handle->Name, address);

        DetourData* detour = NULL;
        for (uint32_t i = 0; i < gDetourData->Size(); ++i)
        {
            DetourData* d = gDetourData->Get(i);
            if (d->Address == address)
            {
                detour = d;
                break;
            }
        }

        if (detour == NULL)
        {
            printf("ib: no detour data exists for address, allocating...\n");
            detour = gDetourData->Allocate();
            detour->Address = address;
        }

        // Only have to initialize a hook for this function if the first
        // detour slot doesn't exist.
        if (detour->First == NULL)
        {
            printf("ib: no hooks exist for detour, initializing for 0x%08x\n", address);

            uint32_t first_instruction;
            Ib_Read(address, &first_instruction, sizeof(uint32_t));

            // Replace the function with a call to the first detour,
            // realistically this will cause a problem for unhooking,
            // but I don't even support that right now.
            Hook(address, handle->Function);

            // Could probably just use an absolute branch since the TOC should be
            // set already from the previous function call, but to be safe might as well
            // use the conditional register.
            uint32_t shellcode[] =
            {
                LIS(0, ((address + 4) >> 16)),
                ORI(0, 0, ((address + 4) & 0xffff)),
                0x7c0903a6, // mtctr %r0
                first_instruction,
                0x4e800420, // bctr
            };

            void* stub_fn = Shellcode(shellcode, sizeof(shellcode));
            ((uint32_t*)handle->Original)[0] = (uint32_t)stub_fn;

            detour->First = handle;
        }
        else
        {
            printf("ib: hook already exists, appending to detour list\n");

            // Otherwise we can just attach this handle to the end of the detour list.
            DetourHandle* first = detour->First;
            while (first->Next != NULL) first = first->Next;
            first->Next = handle;

            // Copy the modified location of the original function from the first detour handle
            handle->Original = detour->First->Original;
        }

        return gNextHookIndex++;
    }


    HookHandle Hook(uintptr_t address, const void* func)
    {
        uint32_t hook_fn_data = ((uint32_t*)func)[0];
        uint32_t hook_fn_toc = ((uint32_t*)func)[1];

        // This is excessive on the codesize,
        // but I don't really care too much honestly.
        uint32_t shellcode[] =
        {
            // hack for hooking function imports
            // 0xf8410028, // std %r2, 0x28(%r1)
            
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

        void* stub_fn = Shellcode(shellcode, sizeof(shellcode));
        if (stub_fn == NULL)
            return INVALID_HOOK_HANDLE;
        
        uint32_t branch = B(stub_fn, address);
        Ib_Write(address, &branch, sizeof(uint32_t));

        return gNextHookIndex++;
    }

    void PokeBranch(uintptr_t address, uintptr_t destination)
    {
        Ib_Poke32(address, B(destination, address));
    }

    void Initialize(InitArgs* args)
    {
        ProcessID = sys_process_getpid();

        if (!args) return;

        gDetourData = args->Detours;
        gBranchPool = args->ExecutableData;
        WriteCache = (EmulatorWriteCache*)args->WriteCache;
        
        while (gStaticDetourHead != NULL)
        {
            Hook(gStaticDetourHead);
            gStaticDetourHead = gStaticDetourHead->PendingInit;
        }

    }
}