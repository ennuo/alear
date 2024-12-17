#include "gooey/GooeyImage.h"
#include "gooey/GooeyNodeManager.h"

#include "ResourceSystem.h"

#include <new>
#include <hook.h>

const u32 DEFAULT_FRAME_DELAY = 2;

const u32 GT_IMAGE = 6;

void CGooeyImage::UpdateAnimatedTexture()
{
    // if (GetType() == GT_IMAGE) return;

    if (!AnimatedTexture || !AnimatedTexture->IsLoaded()) return;

    if (Manager->CurrentHighlightNode != this)
    {
        FrameTimer = 0;
        CurrentFrame = 0;
    }
    else
    {
        FrameTimer++;
        if (FrameTimer >= FrameDelay)
        {
            FrameTimer = 0;
            CurrentFrame = (CurrentFrame + 1) % AnimatedTexture->FrameCount;
        }
    }

    UVs = AnimatedTexture->GetUV(CurrentFrame);
}

MH_DefineFunc(CGooeyImage_InitSDF, 0x00322c88, TOC0, void, CGooeyImage*, CP<RTexture>&, u32, v2, v4, v4);
void CGooeyImage::InitSDF(CP<RTexture>& texture, unsigned int sdf_flags, v2 size, v4 colour, v4 uvs)
{
    CGooeyImage_InitSDF(this, texture, sdf_flags, size, colour, uvs);
}

void CGooeyImage::InitSDF(CP<CResource>& resource, unsigned int sdf_flags, v2 size, v4 colour, v4 uvs)
{
    // if (GetType() == GT_IMAGE)
    {
        if (resource && resource->GetResourceType() == RTYPE_ANIMATED_TEXTURE)
        {
            CP<RAnimatedTexture>& atx = (CP<RAnimatedTexture>&)resource;

            AnimatedTexture = atx;
            atx->BlockUntilLoaded();

            uvs = atx->GetUV(0);

            InitSDF(atx->Texture, sdf_flags, size, colour, uvs);
            
            return;
        }

        AnimatedTexture = NULL;
    }

    if (!resource || resource->GetResourceType() != RTYPE_TEXTURE) resource = NULL;

    CP<RTexture>& texture = (CP<RTexture>&)resource;
    InitSDF(texture, sdf_flags, size, colour, uvs);
}

void CGooeyImage::InitializeExtraData()
{
    new (&AnimatedTexture) CP<RAnimatedTexture>();
    FrameDelay = DEFAULT_FRAME_DELAY;
    CurrentFrame = 0;
    FrameTimer = 0;
}