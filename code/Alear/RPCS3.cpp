#include "RPCS3.h"

#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>

#include <hook.h>
#include <ppcasm.h>

#include <filepath.h>

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

void GeneratePatchYML()
{
    char buf[512];
    FileHandle fd;

    CFilePath filename(FPR_GAMEDATA, "output/generated_patch.yml");
    FileOpen(filename, &fd, OPEN_WRITE);

    #define WRITE_STRING(s) FileWrite(fd, (void*)s, StringLength(s));

    const char* header = 
    "PPU-b7d59e2bd332611fa249066dc6646983aad0a02d:\n"
    "  \"Alear Hooks (EBOOT)\":\n"
    "    Games:\n"
    "      \"LittleBigPlanet Debug\":\n"
    "        LBP1DEBUG: [ All ]\n"
    "    Author: \"ennuo\"\n"
    "    Patch Version: 1.0\n"
    "    Patch:\n";

    const char* prx_header = 
    "PRX-aaaaaaaaaaaaaaaaaaaaaaaaaaaa-0:\n"
    "  \"Alear Hooks (SPRX)\":\n"
    "    Games:\n"
    "      \"LittleBigPlanet Debug\":\n"
    "        LBP1DEBUG: [ All ]\n"
    "    Author: \"ennuo\"\n"
    "    Patch Version: 1.0\n"
    "    Patch:\n";

    WRITE_STRING("Version: 1.2\n\n");
    WRITE_STRING(header);

    // The giant piece of stub data I allocated apparently
    // is the first thing that gets compiled into the ELF so...
    uintptr_t prx_base = (uintptr_t)gStubBaseAddress;
    for (int i = 0; i < gNumWrites; ++i)
    {
        ps3_write& write = gWriteCache[i];
        if (write.Address < prx_base)
        {
            sprintf(buf, "      - [ be32, 0x%x, 0x%08x ]\n", write.Address, write.Word);
            WRITE_STRING(buf);
        }
    }

    WRITE_STRING("\n");

    WRITE_STRING(prx_header);
    for (int i = 0; i < gNumWrites; ++i)
    {
        ps3_write& write = gWriteCache[i];
        if (write.Address >= prx_base)
        {
            sprintf(buf, "      - [ be32, 0x%08x, 0x%08x ]\n", write.Address - prx_base, write.Word);
            WRITE_STRING(buf);
        }
    }

    #undef WRITE_STRING

    FileClose(&fd);
}