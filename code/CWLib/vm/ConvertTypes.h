#ifndef VM_CONVERT_TYPES_H
#define VM_CONVERT_TYPES_H


#include "vm/ScriptContext.h"
#include "vm/ScriptObject.h"

template <typename Type>
struct SConvertScriptTypes
{
    typedef Type NativeType;
    typedef Type VMType;

    static void VMToNative(NativeType* out, CScriptContext* context, VMType* in)
    {
        *out = *in;
    }

    static void NativeToVM(VMType* out, CScriptContext* context, NativeType* in)
    {
        *out = *in;
    }
};

// template <typename Type> struct SConvertScriptTypes;

template <>
struct SConvertScriptTypes<int>
{
    typedef int NativeType;
    typedef int VMType;

    static void VMToNative(NativeType* out, CScriptContext* context, VMType* in)
    {
        *out = *in;
    }

    static void NativeToVM(VMType* out, CScriptContext* context, NativeType* in)
    {
        *out = *in;
    }
};

template <>
struct SConvertScriptTypes<const char*>
{
    typedef const char* NativeType;
    typedef ScriptObjectUID VMType;

    static void VMToNative(NativeType* out, CScriptContext* context, VMType* in)
    {
        // todo: add this later when needed
        *out = NULL;
    }

    static void NativeToVM(VMType* out, CScriptContext* context, NativeType* in)
    {
        out->UID = gScriptObjectManager->RegisterStringA(*in).UID;
    }
};

template <>
struct SConvertScriptTypes<CThing*>
{
    typedef CThing* NativeType;
    typedef ScriptThingUID VMType;

    static void VMToNative(NativeType* out, CScriptContext* context, VMType* in)
    {
        *out = context->LookupThing(*in);
    }

    static void NativeToVM(VMType* out, CScriptContext* context, NativeType* in)
    {
        out->UID = (*in)->UID;
    }
};

#endif // VM_CONVERT_TYPES_H