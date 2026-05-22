#ifndef VM_VIRTUAL_MACHINE_H
#define VM_VIRTUAL_MACHINE_H

#include "vm/ScriptVariant.h"
#include "vm/VMTypes.h"
#include "vm/ScriptArguments.h"
#include "vm/ScriptObject.h"
#include "thing.h"

namespace NVirtualMachine 
{
    extern CScriptVariant (*ExecuteNonStatic)(CThing* this_ptr, const CSignature& signature, const CScriptArguments& args, bool suppress_assert_if_missing, bool* found);
    extern CScriptVariant (*ExecuteStatic)(CThing* world_thing, int script_file, const CSignature& signature, const CScriptArguments& args, bool suppress_assert_if_missing, bool* found);
}




#endif // VM_VIRTUAL_MACHINE_H