#include "vm/ScriptVariant.h"
#include "vm/ScriptObject.h"

#include <thing.h>

CScriptVariant::CScriptVariant(CScriptObject* value)
{
    MachineType = VMT_OBJECT_REF;
    ScriptUID = value != NULL ? value->GetUID() : 0;
}

CScriptVariant::CScriptVariant(const CThing* thing)
{
    MachineType = VMT_SAFE_PTR;
    ScriptUID = thing != NULL ? thing->UID : 0;
}