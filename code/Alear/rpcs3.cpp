#include "rpcs3.h"

#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>

#include "hook.h"
#include "ppcasm.h"

static bool g_IsRPCS3 = false;
bool IsRPCS3()
{
    static bool checked = false;
    if (!checked)
    {
        g_IsRPCS3 = sys_process_getpid() < 0x1000;
        checked = true;
    }

    return g_IsRPCS3;
}

bool IsUsingLLVM()
{
    static bool checked = false;
    if (!checked)
    {
        // Fun little gimmick, overwrite the function we're currently running!!
        u32 shellcode[] =
        {
            LI(3, 0),
            BLR
        };

        MH_Write(*(uint32_t*)&IsUsingLLVM, shellcode, sizeof(shellcode));

        checked = true;
        return IsUsingLLVM();
    }

    return IsRPCS3();
}