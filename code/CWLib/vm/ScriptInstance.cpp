#include <vm/ScriptObject.h>
#include <ResourceScript.h>
#include <PartPhysicsWorld.h>

CSignature gInitSig(".init__");
CSignature gCtorSig(".ctor__");
CSignature gStaticInitSig(".static__");

CScriptInstance::CScriptInstance() : Script(), InstanceLayout(), MemberVariables() // 22
{

}

CScriptInstance::CScriptInstance(const CP<RScript>& script) : // 31
Script(script), InstanceLayout(), MemberVariables()
{

}

CScriptInstance::~CScriptInstance() // 40
{

}
