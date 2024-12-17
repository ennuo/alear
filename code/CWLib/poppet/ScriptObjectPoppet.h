#ifndef POPPET_SCRIPT_OBJECT_POPPET_H
#define POPPET_SCRIPT_OBJECT_POPPET_H

#include "vm/ConvertTypes.h"
#include "scriptobjects/ScriptObjectCustom.h"
#include "Poppet.h"

typedef CScriptObjectCustom<CPoppet> CScriptObjectPoppet;

template <>
struct SConvertScriptTypes<CScriptObjectPoppet*>
{
    typedef CScriptObjectPoppet* NativeType;
    typedef ScriptObjectUID VMType;

    static void VMToNative(NativeType& out, CScriptContext* context, VMType& in)
    {
        out = (NativeType)gScriptObjectManager->LookupInstance(in);
    }

    static void NativeToVM(VMType& out, CScriptContext* context, NativeType& in)
    {
        out.UID = in->GetUID();
    }
};


#endif // POPPET_SCRIPT_OBJECT_POPPET_H
