#include "MemoryTexture.h"

#include "GFXApi.h"
#include "hook.h"


MH_DefineFunc(CMemoryTexture_CMemoryTexture, 0x001d1464, TOC0, void, CMemoryTexture*);
CMemoryTexture::CMemoryTexture()
{
    CMemoryTexture_CMemoryTexture(this);
}

// CMemoryTexture::CMemoryTexture() :
// Texture(), Settings(), Handle(), LocalTextureSize(0), LocalTexture()
// {
//     Settings.Gamma = 0;
//     Settings.MinFilter = CELL_GCM_TEXTURE_LINEAR;
//     Settings.Enable = 1;
//     Settings.WrapR = CELL_GCM_TEXTURE_WRAP;
//     Settings.WrapS = CELL_GCM_TEXTURE_WRAP;
//     Settings.WrapT = CELL_GCM_TEXTURE_WRAP;
//     Settings.URemap = 0;
//     Settings.ZFunc = CELL_GCM_TEXTURE_ZFUNC_LESS;
//     Settings.MagFilter = CELL_GCM_TEXTURE_LINEAR;
//     glFormat = 0;
// }

MH_DefineFunc(CMemoryTexture_BindTexture, 0x001d142c, TOC0, void, CMemoryTexture*, u32);
void CMemoryTexture::BindTexture(u32 sampler)
{
    CMemoryTexture_BindTexture(this, sampler);
    // NGfx::BindTexture(sampler, &Texture, &Settings);
}

MH_DefineFunc(CMemoryTexture_Create, 0x001d1e70, TOC0, void, CMemoryTexture*, CGfxMemoryPool&, u32, u32, u32, u32, u32, u32, u32, u32);
void CMemoryTexture::Create(CGfxMemoryPool& pool, u32 glFormat, u32 width, u32 height, u32 depth, u32 miplevels, u32 swizzled, u32 normalized_uv, u32 surfaces)
{
    CMemoryTexture_Create(this, pool, glFormat, width, height, depth, miplevels, swizzled, normalized_uv, surfaces);
}

MH_DefineFunc(CMemoryTexture_GetOffset, 0x001d12d8, TOC0, u32, CMemoryTexture*, u32, u32);
u32 CMemoryTexture::GetOffset(u32 surface, u32 mip)
{
    return CMemoryTexture_GetOffset(this, surface, mip);
}