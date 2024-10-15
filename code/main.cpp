#include <sys/prx.h>
#include <sys/tty.h>
#include <sys/syscall.h>

#include "alear.h"

SYS_MODULE_INFO(Alear, 0, ALEAR_MAJOR_VERSION, ALEAR_MINOR_VERSION);
SYS_MODULE_START(_start);
SYS_MODULE_STOP(_stop);

extern "C" int _start()
{
    AlearStartup();
    return SYS_PRX_START_OK;
}

extern "C" int _stop()
{
    AlearShutdown();
    return SYS_PRX_STOP_OK;
}