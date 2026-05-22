#include "vm/VirtualMachine.h"


namespace NVirtualMachine
{
    MH_DefineFunc(ExecuteNonStatic, 0x00194674, TOC0, CScriptVariant, CThing* this_ptr, const CSignature& signature, const CScriptArguments& args, bool suppress_assert_if_missing, bool* found);
    MH_DefineFunc(ExecuteStatic, 0x001948c4, TOC0, CScriptVariant, CThing* world_thing, int script_file, const CSignature& signature, const CScriptArguments& args, bool suppress_assert_if_missing, bool* found);
}