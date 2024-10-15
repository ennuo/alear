#ifndef RESOURCE_GFX_TEXTURE_H
#define RESOURCE_GFX_TEXTURE_H

#include <cell/gcm.h>

#include "GfxPool.h"
#include "gfxcore.h"
#include "Resource.h"
#include "TextureType.h"

class RTexture : public CResource {
private:
    u32 DeterministicWidth;
    u32 DeterministicHeight;
    CellGcmTexture gcmtexture;
    CGCMTextureState settings;
    CGfxHandle handle;
    ETextureType texture_type;
    u32 SizeOnDisk;
    u32 LastBoundFrame;
};


#endif // RESOURCE_GFX_TEXTURE_H