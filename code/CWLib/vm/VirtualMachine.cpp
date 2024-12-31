#include "vm/VirtualMachine.h"
#include <hook.h>

namespace NVirtualMachine
{
    MH_DefineFunc(ExecuteNonStatic, 0x00194674, TOC0, CScriptVariant, CThing* this_ptr, CSignature& signature, CScriptArguments& args, bool suppress_assert_if_missing, bool* found);
}