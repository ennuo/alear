#include "PartScript.h"
#include <hook.h>

MH_DefineFunc(PScript_InvokeSync, 0x00028ea4, TOC0, bool, PScript*, CSignature const&, CScriptArguments const&);
bool PScript::InvokeSync(CSignature const& signature, CScriptArguments const& args)
{
    return PScript_InvokeSync(this, signature, args);
}