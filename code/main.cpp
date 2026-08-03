#include <sys/prx.h>

#include <Alear.h>
#include <Ib/Emu.h>

SYS_MODULE_INFO(Alear, 0, ALEAR_MAJOR_VERSION, ALEAR_MINOR_VERSION);
SYS_MODULE_START(_start);
SYS_MODULE_STOP(_stop);

typedef void (*func_ptr) (void);
extern func_ptr __CTOR_LIST__[];
extern func_ptr __CTOR_END__[];

static char gStubData[8192] __attribute__((section(".text#"))) = { 0 };
static DetourData gDetourData[512] = { 0 };
static Ib::EmulatorWriteCache gWriteCache;

static Ib::DirtyAllocator<DetourData> gDetourAllocator("DETOUR_ALLOCATOR", gDetourData, ARRAY_LENGTH(gDetourData));
static Ib::DirtyAllocator<char> gStubDataAllocator("STUB_ALLOCATOR", gStubData, ARRAY_LENGTH(gStubData));

extern "C" int _start()
{
    __SIZE_TYPE__ nptrs = ((__SIZE_TYPE__)__CTOR_END__ - (__SIZE_TYPE__)__CTOR_LIST__) / sizeof(__SIZE_TYPE__);
    for (unsigned i = 0; i < nptrs; ++i)
        __CTOR_LIST__[i]();

    Ib::InitArgs args;
    memset(gDetourData, 0, sizeof(gDetourData));
    memset(&args, 0, sizeof(Ib::InitArgs));
    args.ExecutableData = &gStubDataAllocator;
    args.Detours = &gDetourAllocator;
    if (Ib::IsEmulator())
        args.WriteCache = &gWriteCache;
    Ib::Initialize(&args);

    
    AlearStartup();

    return SYS_PRX_START_OK;
}

extern "C" int _stop()
{
    AlearShutdown();
    return SYS_PRX_STOP_OK;
}

extern "C" void __cxa_pure_virtual()
{

}