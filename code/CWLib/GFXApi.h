#ifndef GFX_API_H
#define GFX_API_H

#include <cell/gcm.h>

#include "gfxcore.h"
#include "GfxPool.h"

namespace NGfx
{
    extern void(*BindTexture)(u32 index, CellGcmTexture* texture, CGCMTextureState* settings);
    extern void*(*tgStart)(u32 numverts, u32 vertexstride);
    extern void(*tgAddVertex)(v4 pos, u32 col, float u, float v, float s, float t);
    extern void(*tgDraw)(u32 primtype, m44 const* transformby, bool setvshader, u32 pshadertype, u32 cameratype, bool tgend, v4 color);
};

#endif // GFX_API_H