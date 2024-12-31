#ifndef PART_LEVEL_SETTINGS_H
#define PART_LEVEL_SETTINGS_H

#include <vector.h>
#include <MMString.h>

#include "Part.h"

class CLevelSettings {
public:
    v4 SunColor;
    v4 FogColor;
    v4 RimColor;
    v4 RimColor2;
    v4 AmbientColor;
    v2 SunPosition;
    float SunMultiplier;
    float FogNear;
    float FogFar;
    float Exposure;
    float SunPositionScale;
    float Weight;
};

class PLevelSettings : public CPart, public CLevelSettings {
public:
    CVector<CLevelSettings> Presets;
    float FromPresetMix;
    float TargetPresetMix;
    float FromFogFactor;
    float TargetFogFactor;
    float FromDarknessFactor;
    float TargetDarknessFactor;
    v4 FromFogColour;
    v4 CurrFogColour;
    v4 TargetFogColour;
    float TargetFogTintColour;
    float TargetFogTintFactor;
    float TargetColourCorrection;
    float CurrBlendFactor;
    bool PresetDirty;
    MMString<char> BackdropAmbience;
};

#endif // PART_LEVEL_SETTINGS_H