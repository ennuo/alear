#ifndef MEMORY_TEXTURE_H
#define MEMORY_TEXTURE_H

#include <cell/gcm.h>

#include "mmtypes.h"

#include "vector.h"

#include "gfxcore.h"
#include "GfxPool.h"

class CMemoryTexture {
public:
    CMemoryTexture();

    void BindTexture(u32 sampler);
    void Create(CGfxMemoryPool& pool, u32 glFormat, u32 width, u32 height, u32 depth, u32 miplevels, u32 swizzled, u32 normalized_uv, u32 surfaces);
    u32 GetOffset(u32 surface, u32 mip);
public:
    CellGcmTexture Texture;
    CGCMTextureState Settings;
    CGfxHandle Handle;
    u32 glFormat;
    u32 LocalTextureSize;
    CRawVector<unsigned char> LocalTexture;
};

#endif // MEMORY_TEXTURE_H