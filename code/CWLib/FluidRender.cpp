#include "FluidRender.h"

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>

MH_DefineFunc(CFluidRender_AddPaintSplatBits, 0x001c9a1c, TOC0, void, m44, v4);
MH_DefineFunc(CFluidRender_AddExplosionBits, 0x001c23d0, TOC0, void, m44);
MH_DefineFunc(CFluidRender_AddFluidBlob, 0x001c03d8, TOC0, void, v4, v4, u32, v4, v4, u32, float);

void CFluidRender::AddPaintSplatBits(m44 pos, v4 color)
{
    CFluidRender_AddPaintSplatBits(pos, color);
}
void CFluidRender::AddExplosionBits(m44 pos)
{
    CFluidRender_AddExplosionBits(pos);
}
void CFluidRender::AddFluidBlob(v4 pos, v4 vel, u32 player, v4 c, v4 d, u32 color_frame, float color_radius)
{
    CFluidRender_AddFluidBlob(pos, vel, player, c, d, color_frame, color_radius);
}