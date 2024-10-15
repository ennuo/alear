#include "ScriptContext.h"
#include "hook.h"

MH_DefineFunc(CScriptContext_LookupThing, 0x00187a44, TOC0, CThing*, CScriptContext*, ScriptThingUID);
CThing* CScriptContext::LookupThing(ScriptThingUID uid)
{
    return CScriptContext_LookupThing(this, uid);
}