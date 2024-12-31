#include "gfxcore.h"

#include <cell/gcm.h>
#include <hook.h>

namespace NGfx 
{
    MH_DefineFunc(FlushGPU, 0x003e2630, TOC1, void, bool wait);

    // void FlushGPU(bool wait)
    // {
    //     extern u32 count;

    //     if (wait)
    //     {
    //         cellGcmFinish(gCellGcmCurrentContext, ++count);
    //         return;
    //     }

    //     cellGcmFlush(gCellGcmCurrentContext);
    // }
};