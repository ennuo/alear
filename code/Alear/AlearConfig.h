#ifndef ALEAR_CONFIG_H
#define ALEAR_CONFIG_H

#include <printf.h>
#include <map>

#include <refcount.h>
#include <mem_allocator.h>
#include <mem_stl_buckets.h>

#include <ResourceTranslationTable.h>
#include <MMString.h>
#include <TextRange.h>

typedef void (*DebugFn)(void);

enum EConfigOptionType {
    OPT_INVALID,
    OPT_BOOL,
    OPT_FLOAT,
    OPT_FUNCTION
};

class CConfigOption {
protected:
    inline CConfigOption(EConfigOptionType type, const char* path, const char* ini_file_name) :
    Invoke(), Next(), Sibling(), Path(path), IniFileName(ini_file_name), DisplayName(), Type(type)
    {
        AddToRegistry();
    }
public:
    inline CConfigOption* GetNext() { return Next; }
    inline CConfigOption* GetSibling() { return Sibling; }
    inline wchar_t* GetDisplayName() { return DisplayName.c_str(); }
    inline const char* GetPath() const { return Path; }
    inline const char* GetIniFileName() const { return IniFileName; }
    inline EConfigOptionType GetType() const { return Type; }
protected:
    void AddToRegistry();
protected:
    DebugFn Invoke;
private:
    CConfigOption* Next;
public:
    CConfigOption* Sibling;
private:
    const char* Path;
    const char* IniFileName;
public:
    MMString<wchar_t> DisplayName;
private:
    EConfigOptionType Type;
};

class CConfigFunction : public CConfigOption {
public:
    inline CConfigFunction(const char* path, DebugFn invoke)
    : CConfigOption(OPT_FUNCTION, path, NULL)
    {
        Invoke = invoke;
    }
public:
    void PerformAction()
    {
        if (Invoke != NULL)
            Invoke();
    }
};

class CConfigBool : public CConfigOption {
public:
    inline CConfigBool(const char* path, const char* ini, bool default_value) 
    : CConfigOption(OPT_BOOL, path, ini), Value(default_value)
    {}

    inline CConfigBool(const char* path, const char* ini, bool default_value, DebugFn invoke) 
    : CConfigOption(OPT_BOOL, path, ini), Value(default_value)
    {
        Invoke = invoke;
    }

public:
    inline bool& operator&() { return Value; }
    inline operator bool() const { return Value; }
    inline CConfigBool& operator=(bool rhs) 
    { 
        Value = rhs;
        if (Invoke != NULL) Invoke();
        return *this; 
    }
private:
    bool Value;
};

class CConfigFloat : public CConfigOption {
public:
    inline CConfigFloat(const char* path, const char* ini, float default_value) : 
    CConfigOption(OPT_FLOAT, path, ini), Value(default_value)
    {
        MinValue = NAN;
        MaxValue = NAN;
        Step = 1.0f;
    }

    inline CConfigFloat(const char* path, const char* ini, float default_value, float min_value, float max_value, float step) : 
    CConfigOption(OPT_FLOAT, path, ini), Value(default_value),
    MinValue(min_value), MaxValue(max_value), Step(step)
    {
    }

    inline CConfigFloat(const char* path, const char* ini, float default_value, float min_value, float max_value, float step, DebugFn invoke) : 
    CConfigOption(OPT_FLOAT, path, ini), Value(default_value),
    MinValue(min_value), MaxValue(max_value), Step(step)
    {
        Invoke = invoke;
    }
public:
    inline float& operator&() { return Value; }
    inline operator float() const { return Value; }
    inline CConfigFloat& operator=(float rhs) 
    { 
        if (!isnan(MaxValue)) rhs = MIN(rhs, MaxValue);
        if (!isnan(MinValue)) rhs = MAX(rhs, MinValue);

        Value = rhs;
        if (Invoke != NULL) Invoke();
        return *this; 
    }

    inline float Decrement() { *this = Value - Step; return Value; }
    inline float Increment() { *this = Value + Step; return Value; }

    inline float GetMinValue() { return MinValue; }
    inline float GetMaxValue() { return MaxValue; }
    inline float GetStep() { return Step; }
private:
    float Value;
    float MinValue;
    float MaxValue;
    float Step;
};

class CConfigFolder {
public:
    CConfigFolder();

    MMString<wchar_t> DisplayName;
    TextRange<char> Path;
    CConfigOption* Options;
    u8 NextSibling;
    u8 FirstChild;
    u8 Parent;
    bool Open;
};

extern CConfigFolder gConfigRoot[];

extern CConfigBool gUseDivergenceCheck;
extern CConfigBool gUsePopitGradients;
extern CConfigBool gUseCustomCursors;
extern CConfigBool gColorCustomCursors;
extern CConfigBool gHideMSPF;
extern CConfigBool gResetLevelSettings;
extern CConfigBool gLoadDefaultMaterial;
extern CConfigBool gForceGFXShapes;
extern CConfigBool gPhysicsToGFX;
extern CConfigBool gForcePlainGFX;
extern CConfigBool gForcePlainBevels;
extern CConfigBool gUnlethalizeAllLethals;
extern CConfigBool gForceMeshGFX;
extern CConfigBool gLoadRemoveAllStickers;
extern CConfigBool gLoadRemoveAllDecorations;
extern CConfigBool gLoadRemoveAllLights;
extern CConfigBool gForceLoadEditable;
extern CConfigBool gSeparateToys;
extern CConfigBool gUseToysPage;
extern CConfigBool gUsePaintPage;
extern CConfigBool gAllowDebugTooltypes;
extern CConfigBool gAllowMeshScaling;
extern CConfigBool gAllowEyedroppingMeshes;
extern CConfigBool gEnablePodTransitionMask;
extern CConfigBool gDebugMaterialTweaks;
extern CConfigBool gUseLegacyKeyColors;
extern CConfigBool gUseNewKeyColorSelection;
extern CConfigBool gCanCollapseCategories;
extern CConfigBool gCanHidePopit;
extern CConfigBool gUseAlternateJointMeshes;
extern CConfigBool gFixupEmitters;
extern CConfigBool gRemoveBurningScripts;

extern CConfigBool gUseIceAccessibility;
extern CConfigBool gPlayBackgroundStings;

extern bool gPauseGameSim;
extern float gFarDist;

void AlearInitConf();


namespace alear
{
    void LoadConfig();
    void SaveConfig();
}

#endif // ALEAR_CONFIG_H