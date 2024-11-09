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
    inline CConfigOption(EConfigOptionType type, const wchar_t* category, const wchar_t* name) : Next(NULL), Category(category), DisplayName(name), Type(type)
    {
        AddToRegistry();
    }
public:
    inline CConfigOption* GetNext() { return Next; }
    inline wchar_t* GetDisplayName() { return (wchar_t*)DisplayName; }
protected:
    void AddToRegistry();
private:
    CConfigOption* Next;
    const wchar_t* Category;
    const wchar_t* DisplayName;
    EConfigOptionType Type;
};

class CConfigBool : public CConfigOption {
public:
    inline CConfigBool(const wchar_t* category, const wchar_t* name, bool default_value) : CConfigOption(OPT_BOOL, category, name), Value(default_value)
    {}
public:
    inline bool& operator&() { return Value; }
    inline operator bool() const { return Value; }
    inline CConfigBool& operator=(bool rhs) { Value = rhs; return *this; }
private:
    bool Value;
};

class CConfigFloat : public CConfigOption {
public:
    inline CConfigFloat(const wchar_t* category, const wchar_t* name, float default_value) : CConfigOption(OPT_FLOAT, category, name), Value(default_value)
    {}
public:
    inline float& operator&() { return Value; }
    inline operator float() const { return Value; }
    inline CConfigFloat& operator=(float rhs) { Value = rhs; return *this; }
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
extern float gFarDist;

extern StaticCP<RTranslationTable> gAlearTrans;

void AlearInitConf();

#endif // ALEAR_CONFIG_H