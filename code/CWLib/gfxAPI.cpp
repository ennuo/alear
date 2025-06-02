#include "GFXApi.h"

#include "hook.h"

namespace NGfx 
{
    MH_DefineFunc(BindTexture, PORT_BIND_TEXTURE, TOC0, void, u32 index, CellGcmTexture* texture, CGCMTextureState* settings);
    MH_DefineFunc(tgStart, 0x001cb140, TOC0, void*, u32 numverts, u32 vertexstride);
    MH_DefineFunc(tgAddVertex, 0x001ca950, TOC0, void, v4 pos, u32 col, float u, float v, float s, float t);
    MH_DefineFunc(tgDraw, 0x001cbc48, TOC0, void, u32 primtype, m44 const* transformby, bool setvshader, u32 pshadertype, u32 cameratype, bool tgend, v4 color);
    MH_DefineFunc(BindShader, 0x001cb8b4, TOC0, void, RShader* shader, bool update_camera_consts_now);

    ESimpleMemPool GetCurDynamicNormalPool()
    {
        return (ESimpleMemPool)(gCurDynamicNormalPool + SMEM_POOL_MAIN_MAPPED_DYNAMIC_GEOM_NOR);
    }
};

MH_DefineFunc(RenderFullscreenQuad, 0x001dd034, TOC0, void, unsigned int col, bool set_pshader, bool set_vshader, float zval);
