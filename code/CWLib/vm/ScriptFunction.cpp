#include "vm/ScriptFunction.h"
#include <hook.h>

namespace NVirtualMachine 
{
    CScriptFunctionBinding::CScriptFunctionBinding() : Script(), FunctionIdx(-1)
    {
        
    }

    void CScriptFunctionBinding::Clear()
    {
        Script = NULL;
        FunctionIdx = -1;
    }

    bool CScriptFunction::Bind()
    {
        if (!Script || Signature.IsEmpty())
        {
            Binding.Clear();
            return false;
        }

        if (Script->LookupFunction(Signature, &Binding)) return true;

        Binding.Clear();
        return false;
    }

    bool CScriptFunction::Rebind() { return Bind(); }


    void UpdateAllScriptFunctionBindings()
    {
        RegisteredScriptFunctions::iterator it;
        for (it = gRegisteredScriptFunctions.begin(); it != gRegisteredScriptFunctions.end(); ++it)
        {
            CScriptFunction* f = *it;
            f->Rebind();
        }
    }
}