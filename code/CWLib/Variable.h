#pragma once

#include <map>

#include <ResourceDescriptor.h>
#include <Resource.h>
#include <SerialiseEnums.h>
#include <ReflectionVisitable.h>
#include <MMString.h>
#include <vector.h>
#include <Serialise.h>
#include <GuidHashMap.h>

#include "hack_thingptr.h"

#include <AlearSR.h>
#include <PoppetOutlineShapes.h>
#include <RenderJoint.h>

class PWorld;
class CThing;
class CResource;

enum EGatherType {
    GATHER_TYPE_GATHER,
    GATHER_TYPE_SAVE,
    GATHER_TYPE_LOAD
};

enum EVariableType {
    VARIABLE_TYPE_NUL=0,
    VARIABLE_TYPE_U8=1,
    VARIABLE_TYPE_U16=2,
    VARIABLE_TYPE_U32=3,
    VARIABLE_TYPE_U64=4,
    VARIABLE_TYPE_FLOAT=5,
    VARIABLE_TYPE_BOOL=6,
    VARIABLE_TYPE_FLOAT_IN_V2=7,
    VARIABLE_TYPE_V2=8,
    VARIABLE_TYPE_V4=9,
    VARIABLE_TYPE_M44=10,
    VARIABLE_TYPE_STRING=11,
    VARIABLE_TYPE_WSTRING=12,
    VARIABLE_TYPE_STRUCT=13,
    VARIABLE_TYPE_THING=14,
    VARIABLE_TYPE_THINGPTR=15,
    VARIABLE_TYPE_RESOURCEPTR=16,
    VARIABLE_TYPE_PTR=17,
    VARIABLE_TYPE_ARRAY=18,
    VARIABLE_TYPE_FCURVE=19,
    VARIABLE_TYPE_PLAN=20,
    VARIABLE_TYPE_PLANDESC=21,
    VARIABLE_TYPE_RESOURCE=22,
    VARIABLE_TYPE_DESCRIPTOR=23,
    VARIABLE_TYPE_DESCRIPTOR_TYPED_LAST=67,
    VARIABLE_TYPE_EGGLINK=68,
    VARIABLE_TYPE_KEYLINK=69,
    VARIABLE_TYPE_ARRAY_EGGLINK=70,
    VARIABLE_TYPE_ARRAY_KEYLINK=71
};

class CGatherVariables;
typedef ReflectReturn (*ReflectFunctionPtr)(CGatherVariables&, void*);
typedef CReflectionVisitable* (*CreateFunc)();
typedef void (*DeleteFunc)(CReflectionVisitable*);

template <typename T>
class FunctionAdapt {
public:
    static CReflectionVisitable* CreateFn() { return new T(); }
    static void DeleteFn(CReflectionVisitable* ptr) { delete ptr; }
};


template<typename R>
ReflectReturn ReflectGP(R& r, CReflectionVisitable*& d, CreateFunc cf, DeleteFunc df, unsigned int size, bool& add);

template<typename R, typename D>
ReflectReturn Reflect(R& r, D*& d)
{
    if (!r.IsGatherVariables()) return REFLECT_NOT_IMPLEMENTED;

    bool add;
    ReflectReturn ret;

    D* ptr = d;
    ret = ReflectGP(r, (CReflectionVisitable*&)d, FunctionAdapt<D>::CreateFn, FunctionAdapt<D>::DeleteFn, sizeof(D), add);
    d = ptr;

    if (ptr != NULL)
        return Add(r, *d, "->");

    // if (ret == REFLECT_OK && add)
    //     return Add(r, *d, "->");
    
    return ret;
}

class CGatherVariables {
public:
    typedef ReflectReturn (*ReflectFunctionPtr)(CGatherVariables&, void*);
public:
    inline CGatherVariables() :
    Data(NULL), Type(VARIABLE_TYPE_NUL), Purpose(GATHER_TYPE_GATHER), ResourceType(RTYPE_INVALID),
    ReflectFunction(), Visited(), Children(), World(NULL), TempString(), LazyCPPriority(STREAM_PRIORITY_DEFAULT),
    Name(NULL), DynamicName(false)
    {
    }

    inline CGatherVariables(const CGatherVariables& rhs) :
    Data(NULL), Type(VARIABLE_TYPE_NUL), Purpose(GATHER_TYPE_GATHER), ResourceType(RTYPE_INVALID),
    ReflectFunction(), Visited(), Children(), World(NULL), TempString(), LazyCPPriority(STREAM_PRIORITY_DEFAULT),
    Name(NULL), DynamicName(false)
    {
        *this = rhs;
    }

    CGatherVariables& operator=(const CGatherVariables& rhs)
    {
        if (this == &rhs) return *this;
        
        Data = rhs.Data;
        Type = rhs.Type;
        Purpose = rhs.Purpose;
        ResourceType = rhs.ResourceType;
        ReflectFunction = rhs.ReflectFunction;
        Visited = rhs.Visited;
        Children = rhs.Children;
        LazyCPPriority = rhs.LazyCPPriority;

        if (rhs.DynamicName) CopyName(rhs.Name);
        else SetName(rhs.Name);

        return *this;
    }

    inline ~CGatherVariables()
    {
        ClearName();
    }
public:
    inline void Set(char const* name, void* data, EVariableType type, ReflectFunctionPtr fn, CGatherVariables& parent)
    {
        World = parent.World;
        LazyCPPriority = STREAM_PRIORITY_DEFAULT;
        Data = data;
        Type = type;
        Purpose = parent.Purpose;
        ReflectFunction = fn;
        Visited = parent.Visited;
        Name = name;
        DynamicName = false;
        ResourceType = RTYPE_INVALID;
    }

    inline void Set(const char* name, void* data, EVariableType type, ReflectFunctionPtr fn, CGatherVariables& parent, EResourceType resource_type, bool dynamic_name)
    {
        World = parent.World;
        LazyCPPriority = STREAM_PRIORITY_DEFAULT;
        Data = data;
        Type = type;
        Purpose = parent.Purpose;
        ReflectFunction = fn;
        Visited = parent.Visited;
        Name = name;
        DynamicName = dynamic_name;
        ResourceType = resource_type;
    }

    ReflectReturn Expand();
    void Collapse();
    CGatherVariables* GetChild(const char*);
    void TakeReflectionCS();
    bool GetLimitThingRecursion();
    bool GetThingPtrAsUID();
    u32 MakeUID();
    bool CanVisitThing(CThing*);
    void* GetVisited(void*);
    void SetVisited(void*, void*);

    inline CStreamPriority GetLazyCPPriority() const
    {
        return LazyCPPriority;
    }

    inline CStreamPriority* GetLazyCPPriorityPtr()
    {
        return &LazyCPPriority;
    }
    
    inline void SetLazyCPPriority(CStreamPriority prio)
    {
        LazyCPPriority = prio;
    }

    bool GetReflectFast();
    inline bool GetSaving() { return Purpose == GATHER_TYPE_SAVE; }
    inline bool GetLoading() { return Purpose == GATHER_TYPE_LOAD; }
    inline u32 GetRevision() { return gHeadRevision.Revision; }
    inline u16 GetResourceVersion() { return ALEAR_LATEST;}
    inline u32 GetCustomVersion() { return ALEAR_BR1_LATEST; }
    inline void SetResourceVersion(u16 version) {}
    inline void SetCustomVersion(u32 version) {}

    u32 GetBranchDescription();
    u16 GetBranchID();
    u16 GetBranchRevision();

    inline u8 GetCompressionFlags() { return 0; }
    inline void SetCompressionFlags(u8) {}
    inline bool IsGatherVariables() { return true; }
    inline ReflectReturn ReadWrite(void*, int) { return REFLECT_OK; }

    bool RequestToAllocate(u64) { return true; }
    bool AllowNullEntries();
    void RegisterResource(CResource*);

    bool GetString(char* str);

    void AddDependency(CDependencyWalkable*, int, const CHash&, const CGUID&);
    bool ToggleDependencies(bool);
    inline bool GetCompressInts() const { return false; }
    inline bool GetCompressVectors() const { return false; }
    inline bool GetCompressMatrices() const { return false; }

    inline const char* GetName() const
    {
        return Name;
    }

    inline bool HasName() const
    {
        return Name != NULL;
    }

    bool IsName(const char*) const;

    void SetName(const char* name)
    {
        ClearName();
        if (name != NULL)
        {
            Name = name;
            DynamicName = false;
        }
    }

    void CopyName(const char* name)
    {
        ClearName();
        if (name != NULL)
        {
            char* dyn = new char[strlen(name + 1)];
            strcpy(dyn, name);

            Name = dyn;
            DynamicName = true;
        }
    }

    inline void ClearName()
    {
        if (DynamicName)
        {
            if (Name != NULL)
                delete Name;
            Name = NULL;
        }

        DynamicName = false;
        Name = NULL;
    }

    inline CGatherVariables& AddChild() 
    { 
        Children.try_resize(Children.size() + 1);
        return Children[Children.size() - 1];
    }
public:
    void* Data;
    EVariableType Type;
    EGatherType Purpose;
    EResourceType ResourceType;
    ReflectFunctionPtr ReflectFunction;
    void* Visited; // this is a map i just dont care
    CVector<CGatherVariables> Children;
    PWorld* World;
    MMString<char> TempString;
private:
    CStreamPriority LazyCPPriority;
public:
    const char* Name;
    bool DynamicName;
};

template<typename R, typename D>
ReflectReturn ReflectWrapper(R& r, D& d) // 147
{
    return Reflect(r, d);
}

template <typename T, bool some_bool>
class GetReflectFunction {
public:
    static ReflectReturn (*Get())(CGatherVariables&, T&)
    {
        return ReflectWrapper<CGatherVariables, T>;
    }
};

template <typename T> struct variable_type { static const EVariableType value = VARIABLE_TYPE_STRUCT; };
template <typename T> struct variable_type<T*> { static const EVariableType value = VARIABLE_TYPE_PTR; };
template <typename T> struct variable_type<CVector<T> > { static const EVariableType value = VARIABLE_TYPE_ARRAY; };
template <> struct variable_type<MMString<char> > { static const EVariableType value = VARIABLE_TYPE_STRING; };
template <> struct variable_type<MMString<wchar_t> > { static const EVariableType value = VARIABLE_TYPE_WSTRING; };
template <> struct variable_type<MMString<tchar_t> > { static const EVariableType value = VARIABLE_TYPE_WSTRING; };
template <> struct variable_type<s8> { static const EVariableType value = VARIABLE_TYPE_U8; };
template <> struct variable_type<u8> { static const EVariableType value = VARIABLE_TYPE_U8; };
template <> struct variable_type<s16> { static const EVariableType value = VARIABLE_TYPE_U16; };
template <> struct variable_type<u16> { static const EVariableType value = VARIABLE_TYPE_U16; };
template <> struct variable_type<s32> { static const EVariableType value = VARIABLE_TYPE_U32; };
template <> struct variable_type<u32> { static const EVariableType value = VARIABLE_TYPE_U32; };
template <> struct variable_type<f32> { static const EVariableType value = VARIABLE_TYPE_FLOAT; };
template <> struct variable_type<bool> { static const EVariableType value = VARIABLE_TYPE_BOOL; };
template <> struct variable_type<v4> { static const EVariableType value = VARIABLE_TYPE_V4; };
template <> struct variable_type<CThingPtr> { static const EVariableType value = VARIABLE_TYPE_THINGPTR; };
template <> struct variable_type<CGUID> { static const EVariableType value = VARIABLE_TYPE_U32; };

// variable.h: 288, all defined here
template <typename D>
ReflectReturn Add(CGatherVariables& r, D& d, const char* c)
{
    const EVariableType type = variable_type<D>::value;
    ReflectFunctionPtr ptr = NULL;
    if (type >= VARIABLE_TYPE_STRUCT)
        ptr = (ReflectFunctionPtr)GetReflectFunction<D, false>::Get();
    r.AddChild().Set(c, (void*)&d, type, ptr, r);
    return REFLECT_OK;
}

template <typename T>
void Init(CGatherVariables& variables, T* data)
{
    variables.SetName("<Root>");
    variables.ReflectFunction = (ReflectFunctionPtr)GetReflectFunction<T, false>::Get();
    variables.Data = (void*)data;
    variables.Type = VARIABLE_TYPE_STRUCT;
}

extern ReflectReturn (*GatherVariablesLoad)(ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes);
