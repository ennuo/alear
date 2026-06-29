#include <vm/ScriptContext.h>
#include <thing.h>

ScriptThingUID::ScriptThingUID(CThing* thing) { UID = thing->UID; }
ScriptThingUID::ScriptThingUID(const ScriptThingUID& uid) { UID = uid.UID; }
ScriptThingUID::ScriptThingUID(u32 uid) { UID = uid; }

MH_DefineFunc(CScriptContext_LookupThing, 0x00187a44, TOC0, CThing*, CScriptContext*, ScriptThingUID);
CThing* CScriptContext::LookupThing(ScriptThingUID uid)
{
    return CScriptContext_LookupThing(this, uid);
}