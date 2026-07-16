#pragma once

#include <Part.h>
#include <hack_thingptr.h>
class RTexture;

class PSpriteLight : public CPart {
public:
    float GetOnDest() const;
    void UpdateFlicker();
protected:
    v4 Color;
    v4 ColorOverride;
    float Multiplier;
    float GlowRadius;
    float FarDist;
    float SourceSize;
    CP<RTexture> FalloffTexture;
    CThingPtr LookAt;
    float FogAmount;
    bool SpotLight;
    bool EnableFogShadows;
    bool EnableFog;
    float OnDest;
    float OnSpeed;
    float OffSpeed;
    float FlickerProb;
    float FlickerAmount;
    float OnCur;
    float CurFlicker;
    m44 world2light;
    m44 light2world;
    v4 oldlightpos;
    v4 lightpos;
    v4 corners[8];
    v4 consts;
    float ln;
    float lf;
    bool InvertedMatrix;
    bool Enable;
    u32 PixelCountIndex;
    u32 LastPixelCount;
    u32 LastPixelCountGlow;
    u8 CurChannel;
};