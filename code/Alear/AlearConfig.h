#ifndef ALEAR_CONFIG_H
#define ALEAR_CONFIG_H

#include <printf.h>
#include <map>

#include <refcount.h>
#include <mem_allocator.h>
#include <mem_stl_buckets.h>

#include <ResourceTranslationTable.h>

typedef void (*DebugFn)(void);

enum EConfigOptionType {
    OPT_INVALID,
    OPT_BOOL,
    OPT_FLOAT
};

class CConfigOption {
protected:
    inline CConfigOption(EConfigOptionType type, const wchar_t* category, const wchar_t* name) : Next(NULL), Category(category), DisplayName(name), Type(type), Invoke(NULL)
    {
        AddToRegistry();
    }
public:
    inline CConfigOption* GetNext() { return Next; }
    inline wchar_t* GetDisplayName() { return (wchar_t*)DisplayName; }
    inline EConfigOptionType GetType() { return Type; }
protected:
    void AddToRegistry();
protected:
    DebugFn Invoke;
private:
    CConfigOption* Next;
    const wchar_t* Category;
    const wchar_t* DisplayName;
    EConfigOptionType Type;
};

class CConfigBool : public CConfigOption {
public:
    inline CConfigBool(const wchar_t* category, const wchar_t* name, bool default_value) 
    : CConfigOption(OPT_BOOL, category, name), Value(default_value)
    {}

    inline CConfigBool(const wchar_t* category, const wchar_t* name, bool default_value, DebugFn invoke) 
    : CConfigOption(OPT_BOOL, category, name), Value(default_value)
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
    inline CConfigFloat(const wchar_t* category, const wchar_t* name, float default_value) : 
    CConfigOption(OPT_FLOAT, category, name), Value(default_value)
    {
        MinValue = NAN;
        MaxValue = NAN;
        Step = 1.0f;
    }

    inline CConfigFloat(const wchar_t* category, const wchar_t* name, float default_value, float min_value, float max_value, float step) : 
    CConfigOption(OPT_FLOAT, category, name), Value(default_value),
    MinValue(min_value), MaxValue(max_value), Step(step)
    {
    }

    inline CConfigFloat(const wchar_t* category, const wchar_t* name, float default_value, float min_value, float max_value, float step, DebugFn invoke) : 
    CConfigOption(OPT_FLOAT, category, name), Value(default_value),
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

typedef std::map<const wchar_t*, CConfigOption*, std::less<const wchar_t*>, STLBucketAlloc<std::pair<const wchar_t*, CConfigOption*> > > ConfigMap;
extern ConfigMap gConfigMap;

extern CConfigBool gUseDivergenceCheck;
extern CConfigBool gUsePopitGradients;
extern CConfigBool gHideMSPF;
extern CConfigBool gLoadDefaultMaterial;
extern CConfigBool gForceLoadEditable;

extern CConfigBool gUseIceAccessibility;

extern bool gPauseGameSim;
extern float gFarDist;

void AlearInitConf();

#endif // ALEAR_CONFIG_H