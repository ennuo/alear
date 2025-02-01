#include "vm/ScriptVariant.h"
#include "vm/ScriptObject.h"

CScriptVariant::CScriptVariant(CScriptObject* value)
{
    MachineType = VMT_OBJECT_REF;
    ScriptUID = value != NULL ? value->GetUID() : 0;
}