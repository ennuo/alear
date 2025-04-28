#ifndef PART_PHYSICS_WORLD_H
#define PART_PHYSICS_WORLD_H

#include "Part.h"
#include "vector.h"
#include "hack_thingptr.h"
#include "PoppetEnums.inl"

class PPos;
class PRenderMesh;
class PGeneratedMesh;
class PYellowHead;
class PSwitch;

class CColCorrectMatrix {
    v4 Tint;
    float Saturation;
    float Hue;
    float Contrast;
    float Brightness;
};

class CColCorrectSettings {
public:
    CColCorrectMatrix Matrix[2];
    v4 ColPick;
    v4 VignetteTop;
    v4 VignetteBottom;
};

class CGlobalSettings {
public:
    float LightingFactor;
    float ColourCorrectionFactor;
    CColCorrectSettings ColourCorrectionSettings;
    float FogFactor;
    float FogTintFactor;
    float DarknessFactor;
};

enum EOnlineMode {
    MODE_DEFAULT,
    MODE_DEDICATED_DETERMINISTIC,
    MODE_DEDICATED_NON_DETERMINISTIC
};

struct BroadcastMicrochip {
    CThingPtr SourceMicrochip;
    CThingPtr ClonedMicrochip;
};

class RLevel;
class PWorld : public CPart {
public:
    CThing* GetThingByUID(u32 uid);
    CThing* FindThingByScriptName(char const* name);

    void ResetToDefaultGlobalSettings(EGlobalSettingsType type);
    void ClearGlobalSettingUID(EGlobalSettingsType type);
    void InitBackdropCuller();
    
    void UpgradeAllScripts();
    float GetWaterLevelWorldYCoords() const;
public:
    u32 ThingUIDCounter;
    CRawVector<CThing*> Things;
    RLevel* Level;
private:
    char Pad[0x48];
public:
    CRawVector<PRenderMesh*> ListPRenderMesh;
    CRawVector<PPos*> ListPPos;
    CRawVector<void*> ListPTrigger;
    CRawVector<PYellowHead*> ListPYellowHead;
    CRawVector<void*> ListPAudioWorld;
    CRawVector<void*> ListPAnimation;
    CRawVector<PGeneratedMesh*> ListPGeneratedMesh;
private:
    char Pad5[0xb4];
public:
    CRawVector<PSwitch*> ListPSwitch;
private:
    char Pad6[0xc8];
public:
    float LightingFactor; // 0x244 - 580
    float ColorCorrectionFactor;
    float FogFactor;
    float FogTintFactor;
    float DarknessFactor;
    CGlobalSettings TargetGlobalSettings;
    CGlobalSettings FromGlobalSettings;
    bool HasLevelLightingBeenSetup;
    bool HaveGlobalSettingsGuidsBeenSet;
    float WaterLevel; // 900
    float FromWaterLevel; // 904
    float TargetWaterLevel;
    float WaterWaveMagnitude;
    float FromWaterWaveMagnitude;
    float TargetWaterWaveMagnitude;
    float WaterTint;
	v4 FromWaterTintColour;
	float TargetWaterTint;
	float WaterMurkiness;
	float FromWaterMurkiness;
	float TargetWaterMurkiness;
	float WaterBits;
	float FromWaterBits;
	float TargetWaterBits;
    bool IsLocalPlayerTweakingWater;
    float CachedTweakWaterLevel;
    bool CachedTweakedWaterEnabled;
    float Gravity;
    float FromGravity;
    float TargetGravity;
private:
    char Pad8[0x88];
public:
    CThingPtr Backdrop; // 1132 
    CThingPtr BackdropNew; // 1144
private:
    char Pad9[0xa5];
public:
    bool IsTutorialLevel;
private:
    char Pad10[0xc52];
public:
    CVector<BroadcastMicrochip> BroadcastMicrochips;
    u32 GameMode;
    u32 OnlineMode;
    float GameModeScore[4];
    bool UseAdvancedLogicSystem;
};

#endif // PART_PHYSICS_WORLD_H