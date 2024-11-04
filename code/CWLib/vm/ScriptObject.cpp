#include "ScriptObject.h"
#include "hook.h"

MH_DefineFunc(CScriptObjectManager_RegisterStringA, 0x00190950, TOC0, ScriptObjectUID, CScriptObjectManager*, const char*);
ScriptObjectUID CScriptObjectManager::RegisterStringA(const char* string)
{
    return CScriptObjectManager_RegisterStringA(this, string);
}


MH_DefineFunc(CScriptObjectManager_LookupInstance, 0x0018ecc8, TOC0, CScriptObjectInstance*, CScriptObjectManager*, ScriptObjectUID);
CScriptObjectInstance* CScriptObjectManager::LookupInstance(ScriptObjectUID object_uid)
{
    return CScriptObjectManager_LookupInstance(this, object_uid);
}