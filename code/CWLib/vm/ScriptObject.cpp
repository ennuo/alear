#include "ScriptObject.h"
#include "ResourceScript.h"
#include "hook.h"

MH_DefineFunc(CScriptObjectManager_RegisterStringA, 0x00190950, TOC0, ScriptObjectUID, CScriptObjectManager*, const char*);
ScriptObjectUID CScriptObjectManager::RegisterStringA(const char* string)
{
    return CScriptObjectManager_RegisterStringA(this, string);
}

MH_DefineFunc(CScriptObjectManager_RegisterStringW, 0x001926ec, TOC0, ScriptObjectUID, CScriptObjectManager*, const wchar_t*);
ScriptObjectUID CScriptObjectManager::RegisterStringW(const wchar_t* string)
{
    return CScriptObjectManager_RegisterStringW(this, string);
}

MH_DefineFunc(CScriptObjectManager_LookupInstance, 0x0018ecc8, TOC0, CScriptObjectInstance*, CScriptObjectManager*, ScriptObjectUID);
CScriptObjectInstance* CScriptObjectManager::LookupInstance(ScriptObjectUID object_uid)
{
    return CScriptObjectManager_LookupInstance(this, object_uid);
}

MH_DefineFunc(CScriptObjectManager_LookupObject, 0x0018eb28, TOC0, CScriptObject*, CScriptObjectManager*, u32);
CScriptObject* CScriptObjectManager::LookupObject(u32 object_uid)
{
    return CScriptObjectManager_LookupObject(this, object_uid);
}

MH_DefineFunc(CScriptObjectManager_RegisterResource, 0x00190c10, TOC0, ScriptObjectUID, CScriptObjectManager*, CResource*);
ScriptObjectUID CScriptObjectManager::RegisterResource(CResource* res)
{
    return CScriptObjectManager_RegisterResource(this, res);
}

MH_DefineFunc(CScriptObjectInstance_InvokeSync, 0x0018ed54, TOC0, bool, CScriptObjectInstance*, PWorld* pworld, CSignature const&, CScriptArguments const&, CScriptVariant*);
bool CScriptObjectInstance::InvokeSync(PWorld* pworld, CSignature const& signature, CScriptArguments const& arguments, CScriptVariant* return_value)
{
    return CScriptObjectInstance_InvokeSync(this, pworld, signature, arguments, return_value);
}

CScriptObjectInstance::CScriptObjectInstance(CP<RScript>& script) : CScriptObject(), ScriptInstance(script)
{

}

MH_DefineFunc(CScriptObjectInstance_Create, 0x00192658, TOC0, CScriptObjectInstance*, CP<RScript>&, PWorld*, bool);
CScriptObjectInstance* CScriptObjectInstance::Create(CP<RScript>& script, PWorld* pworld, bool default_construct)\
{
    return CScriptObjectInstance_Create(script, pworld, default_construct);
}

MH_DefineFunc(CScriptObjectManager_AddRoot, 0x0018f510, TOC0, void, CScriptObjectManager*, CScriptObject*);
void CScriptObjectManager::AddRoot(CScriptObject* object)
{
    CScriptObjectManager_AddRoot(this, object);
}