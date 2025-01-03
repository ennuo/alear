#ifndef GFX_API_H
#define GFX_API_H

#include <cell/gcm.h>

#include "gfxcore.h"
#include "GfxPool.h"

struct CTGVertex {
    v4 pos;
    float tc0[4];
    float tc1[3];
    u32 col;
};

namespace NGfx
{
    extern CTGVertex* tgVtxCur;
    extern CTGVertex* tgVtxEnd;
    extern bool gTgUseNormals;
    
    extern void(*BindTexture)(u32 index, CellGcmTexture* texture, CGCMTextureState* settings);
    extern void*(*tgStart)(u32 numverts, u32 vertexstride);
    extern void(*tgAddVertex)(v4 pos, u32 col, float u, float v, float s, float t);
    extern void(*tgDraw)(u32 primtype, m44 const* transformby, bool setvshader, u32 pshadertype, u32 cameratype, bool tgend, v4 color);
};

#endif // GFX_API_H