#ifndef RESOURCE_ANIMATED_TEXTURE_H
#define RESOURCE_ANIMATED_TEXTURE_H


#include "refcount.h"
#include "ResourceGFXTexture.h"
#include "Resource.h"

class RAnimatedTexture : public CResource {
public:
    CP<RTexture> Texture;
    u32 FrameCount;
    u32 FrameWidth;
    u32 FrameHeight;
    bool BlendFrames;
};

#endif // RESOURCE_ANIMATED_TEXTURE_H