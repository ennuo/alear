#include "ScriptObject.h"
#include "hook.h"

MH_DefineFunc(CScriptObjectManager_RegisterStringA, 0x00190950, TOC0, ScriptObjectUID, CScriptObjectManager*, const char*);
ScriptObjectUID CScriptObjectManager::RegisterStringA(const char* string)
{
    return CScriptObjectManager_RegisterStringA(this, string);
}