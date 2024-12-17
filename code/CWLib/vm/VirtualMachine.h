#ifndef VM_VIRTUAL_MACHINE_H
#define VM_VIRTUAL_MACHINE_H

#include "vm/ScriptVariant.h"
#include "vm/VMTypes.h"
#include "vm/ScriptArguments.h"
#include "thing.h"

namespace NVirtualMachine 
{
    extern CScriptVariant (*ExecuteNonStatic)(CThing* this_ptr, CSignature& signature, CScriptArguments& args, bool suppress_assert_if_missing, bool* found);
}




#endif // VM_VIRTUAL_MACHINE_H