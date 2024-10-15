#ifndef RESOURCE_GFX_MATERIAL_H
#define RESOURCE_GFX_MATERIAL_H

#include <Cg/NV/cg_types.h>


#include "Resource.h"

class RGfxMaterial : public CResource {
enum {
    SHADER_N,
    SHADER_C,
    SHADER_C_DECAL,
    SHADER_W,

    SHADER_LAST
};
public:
    u32 GMatFlags;
    CGprogram Shaders[SHADER_LAST];
    u32 ShaderBinaryOffsets[SHADER_LAST];
    u8 TextureUsage[SHADER_LAST];
};

#endif // RESOURCE_GFX_MATERIAL_H