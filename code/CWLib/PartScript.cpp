#include "PartScript.h"
#include <hook.h>

#include "vm/VMTypes.h"

MH_DefineFunc(PScript_InvokeSync, 0x00028ea4, TOC0, bool, PScript*, CSignature const&, CScriptArguments const&);
bool PScript::InvokeSync(CSignature const& signature, CScriptArguments const& args)
{
    return PScript_InvokeSync(this, signature, args);
}

MH_DefineFunc(PScript_SetScript, 0x0002d198, TOC0, void, PScript*, CP<RScript> const& script);
void PScript::SetScript(CP<RScript> const& script)
{
    return PScript_SetScript(this, script);
}