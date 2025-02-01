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


// MH_DefineFunc(SetRenderTarget, 0x003e2a00, TOC1, void, u32 xres, u32 yres, u32* rtarget_offset, u32* rtarget_location, u32* rtarget_pitch, u32 num_rt, u32 format);

MH_DefineFunc(SetRenderTarget, 0x003e2db8, TOC1, void, CellGcmSurface& surf);


MH_DefineFunc(SetRenderTargetBackBuffer, 0x003e2f7c, TOC1, void, u32 index);
MH_DefineFunc(SetNiceState, 0x003e2794, TOC1, void);