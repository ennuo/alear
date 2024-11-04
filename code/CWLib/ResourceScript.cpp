#include "ResourceScript.h"
#include <hook.h>

MH_DefineFunc(RScript_LookupFunction, 0x00098e04, TOC0, bool, RScript const*, CSignature const&, NVirtualMachine::CScriptFunctionBinding*);
bool RScript::LookupFunction(CSignature const& signature, NVirtualMachine::CScriptFunctionBinding* binding) const
{
    return RScript_LookupFunction(this, signature, binding);
}

MH_DefineFunc(RScript_Fixup, 0x000cc04c, TOC0, void, RScript*);
void RScript::Fixup()
{
    return RScript_Fixup(this);
}

bool RScript::IsInstanceLayoutValid()
{
    return false;
    // if (FixedUp && ExportsFixedUp)
    // {
    //     if (!SuperClassScript) return true;
    //     if (SuperClassScript->IsInstanceLayoutValid() && SuperInstanceLayout.GetRef() != NULL)
    //         return SuperClassScript->GetInstanceLayout() == SuperInstanceLayout;
    // }

    // return false;
}

void RScript::ForceFixup()
{
    FixedUp = false;
    ExportsFixedUp = false;
    InstanceLayout = NULL;
    SuperInstanceLayout = NULL;
}