#ifndef RESOURCE_GFX_MATERIAL_H
#define RESOURCE_GFX_MATERIAL_H

#include <Cg/NV/cg_types.h>

#include "Resource.h"

class CMaterialBox {
public:
    u32 Type;
    u32 Params[6];
    float x, y, w, h;
};

class CMaterialWire {
public:
    int BoxFrom;
    int BoxTo;
    u8 PortFrom;
    u8 PortTo;
    char Swizzle[5];
    bool error;
};

class CMaterialParameterAnimation {
public:
    inline CMaterialParameterAnimation() : BaseValue(), Keys(), Name(), ComponentsAnimated()
    {

    }
public:
    v4 BaseValue;
    CRawVector<float> Keys;
    char Name[3];
    u8 ComponentsAnimated;
};

class CMaterialBoxAttributes {
public:
    inline CMaterialBoxAttributes() : SubType(0), AnimIndex(-1), SecondaryAnimIndex(-1), ExtraParams()
    {

    }
public:
    u32 SubType;
    u16 AnimIndex;
    u16 SecondaryAnimIndex;
    u32 ExtraParams[2];
};

class RGfxMaterial : public CResource {
enum {
    SHADER_N,
    SHADER_C,
    SHADER_C_DECAL,
    SHADER_W,

    SHADER_LAST
};
public:
    void InitializeExtraData();
    void DestroyExtraData();
public:
    u32 GMatFlags;
    CGprogram Shaders[SHADER_LAST];
    u32 ShaderBinaryOffsets[SHADER_LAST];
    u8 TextureUsage[SHADER_LAST];
    CP<RTexture> Textures[8];
    u8 WrapS[8];
    u8 WrapT[8];
    ByteArray PS3BinaryCode;
    CGfxHandle pshadercode;
    float AlphaTestLevel;
    float BumpLevel;
    float FresnelLevel;
    float ReflectionBlur;
    float RimSoftening;
    u32 SoundEnum;
    u32 LightSettingsHash[4];
    u8 AlphaLayer;
    u8 ShadowCastMode;
    /* sneaking this in the padding */
    bool UsesPlayerDefinedColour;
private:
    char Pad[0x6];
public:
    CVector<CMaterialParameterAnimation> ParameterAnimations;
    CVector<CMaterialBoxAttributes> BoxAttributes;
    u8 AlphaMode;
};

#endif // RESOURCE_GFX_MATERIAL_H