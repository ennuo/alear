#ifndef GOOEY_SCRIPT_INTERFACE_H
#define GOOEY_SCRIPT_INTERFACE_H

#include "vm/ConvertTypes.h"
#include "scriptobjects/ScriptObjectCustom.h"
#include "gooey/ScriptedGooeyWrapper.h"
#include "gooey/GooeyNodeManager.h"

typedef CScriptObjectCustom<CScriptedGooeyWrapper> CScriptObjectGooey;

template <>
struct SConvertScriptTypes<CScriptObjectGooey*>
{
    typedef CScriptObjectGooey* NativeType;
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


#endif // GOOEY_SCRIPT_INTERFACE_H