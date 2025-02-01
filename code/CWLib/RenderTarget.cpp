#include "RenderTarget.h"
#include "GFXApi.h"

u32 CRenderTarget::GetWidth()
{
    return Texture.width >> (u32)(1 < MSAA);
}

u32 CRenderTarget::GetHeight()
{
    return Texture.height >> (u32)(2 < MSAA);
}

void CRenderTarget::BindAsTexture(int texunit, CGCMTextureState* settings)
{
    NGfx::BindTexture(texunit, &Texture, settings);
}