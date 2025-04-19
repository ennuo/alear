#ifndef VARIABLE_H
#define VARIABLE_H

#include "SerialiseEnums.h"
#include "PartPhysicsWorld.h"
#include "ResourceDescriptor.h"
#include <MMString.h>
#include <map>

#include "hack_thingptr.h"

#include <AlearSR.h>
#include <PoppetOutlineShapes.h>
#include <RenderJoint.h>


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

// TEMP SHIT UNTIL I FIGURE OUT THE REFLECTION NONSENSE
#include "customization/SlapStyles.h"
#include "customization/Emotes.h"
#include <cell/DebugLog.h>

// these ones are defined all over the place
template<typename R>
ReflectReturn Reflect(R& r, CSlapMesh& d);
template<typename R>
ReflectReturn Reflect(R& r, CSlapStyles& d);
template<typename R>
ReflectReturn Reflect(R& r, CEmote& d);
template<typename R>
ReflectReturn Reflect(R& r, CEmoteBank& d);
template<typename R>
ReflectReturn Reflect(R& r, CEmoteSound& d);
template<typename R>
ReflectReturn Reflect(R& r, CAnimStyle& d);
template<typename R>
ReflectReturn Reflect(R& r, CStyleBank& d);
template<typename R>
ReflectReturn Reflect(R& r, CThingPtr& d);
template <typename R>
ReflectReturn Reflect(R& r, CPoppetOutlineConfig& d);
template <typename R>
ReflectReturn Reflect(R& r, CPoppetOutline& d);
template <typename R>
ReflectReturn Reflect(R& r, CRenderJoint& d);
template <typename R>
ReflectReturn Reflect(R& r, CRenderJoints& d);


// variable.h: 147
template<typename R, typename D>
ReflectReturn ReflectWrapper(R& r, D& d)
{
    return Reflect(r, d);
}

template <typename D>
ReflectReturn Add(CGatherVariables& r, D& d, char* c);

// variable.h: 884
template<typename R, typename D>
ReflectReturn ReflectVector(R& r, D& d)
{
    ReflectReturn ret;

    u32& size = d.GetSizeForSerialisation();
    if (!r.IsGatherVariables())
    {
        ret = Reflect(r, size);
        if (ret != REFLECT_OK) return ret;
        DebugLog("attempting to serialize vector with %d elements...\n", size);

        if (!r.RequestToAllocate(size * sizeof(D)))
            return REFLECT_EXCESSIVE_ALLOCATIONS;
    }

    u32 len = size;
    
    if ((r.GetLoading() && d.begin() == NULL) || d.begin() == NULL)
    {
        if (size != 0)
        {
            size = 0;
            d.try_resize(len);
        }
    }

    if (r.GetLoading())
    {
        d.clear();
        d.try_resize(len);
    }
    
    for (u32 i = 0; i < size; ++i)
    {
        ret = Add(r, d[i], NULL);
        if (ret != REFLECT_OK) return ret;
    }
    
    return ret;
}

template<typename R, typename D>
ReflectReturn Reflect(R& r, CVector<D>& d)
{
    return ReflectVector<R, CVector<D> >(r, d);
}

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

template <typename T, bool some_bool>
class GetReflectFunction {
public:
    static ReflectReturn (*Get())(CGatherVariables&, T&)
    {
        return ReflectWrapper<CGatherVariables, T>;
    }
};

class CGatherVariables {
public:
    inline CGatherVariables() :
    Data(NULL), Type(VARIABLE_TYPE_NUL), Purpose(GATHER_TYPE_GATHER), ResourceType(RTYPE_INVALID),
    ReflectFunction(), Visited(), Children(), World(NULL), TempString(), LazyCPPriority(STREAM_PRIORITY_DEFAULT),
    Name(NULL), DynamicName(false)
    {

    }

    inline ~CGatherVariables()
    {
        ClearName();
    }
public:
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

    inline u32 GetRevision() { return 0x272; }

    inline u16 GetResourceVersion() { return ALEAR_LATEST_PLUS_ONE - 1;}
    inline u32 GetCustomVersion() { return ALEAR_BR1_LATEST_PLUS_ONE - 1; }

    inline void SetResourceVersion(u16 version) {}
    inline void SetCustomVersion(u32 version) {}

    inline bool IsGatherVariables() { return true; }
    inline bool GetLoading() { return Purpose == GATHER_TYPE_LOAD; }
    inline bool GetSaving() { return Purpose == GATHER_TYPE_SAVE; }
    inline bool GetCompressInts() { return false; }
    inline bool RequestToAllocate(u64 size) { return true; }

    inline ReflectReturn Align(int a) { return REFLECT_OK; }
    inline ReflectReturn CleanupDecompression() { return REFLECT_NOT_IMPLEMENTED; }
    inline ReflectReturn LoadCompressionData(u32* totalsize) { return REFLECT_NOT_IMPLEMENTED; }
    inline u32 GetVecLeft() { return 0; }
    inline u8 GetCompressionFlags() { return 0; }
    inline void SetCompressionFlags(u8 flags) { }
    inline ReflectReturn StartCompressing() { return REFLECT_NOT_IMPLEMENTED; }
    inline ReflectReturn FinishCompressing() { return REFLECT_NOT_IMPLEMENTED; }




    inline ReflectReturn ReadWrite(void*, int) { return REFLECT_OK; }
    inline CGatherVariables& AddChild() 
    { 
        Children.try_resize(Children.size() + 1);
        return Children[Children.size() - 1];
    }

    inline void Set(char const* name, void* data, EVariableType type, ReflectFunctionPtr fn, CGatherVariables& parent)
    {
        World = parent.World;
        LazyCPPriority = STREAM_PRIORITY_DEFAULT;
        Data = data;
        Type = type;
        Purpose = parent.Purpose;
        ReflectFunction = fn;
        Visited = Visited;
        Name = name;
        DynamicName = false;
        ResourceType = RTYPE_INVALID;
    }

    inline void SetName(const char* name)
    {
        ClearName();
        if (name != NULL)
        {
            Name = name;
            DynamicName = false;
        }
    }

    template <typename T>
    void Init(T* data)
    {
        SetName("<Root>");
        Type = VARIABLE_TYPE_STRUCT;
        Data = (void*)data;
        ReflectFunction = (ReflectFunctionPtr)GetReflectFunction<T, false>::Get();
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
    CStreamPriority LazyCPPriority;
    const char* Name;
    bool DynamicName;
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
ReflectReturn Add(CGatherVariables& r, D& d, char* c)
{
    const EVariableType type = variable_type<D>::value;
    ReflectFunctionPtr ptr = NULL;
    if (type >= VARIABLE_TYPE_STRUCT)
        ptr = (ReflectFunctionPtr)GetReflectFunction<D, false>::Get();
    r.AddChild().Set(c, (void*)&d, type, ptr, r);
    return REFLECT_OK;
}

extern ReflectReturn (*GatherVariablesLoad)(ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes);

#endif // VARIABLE_H