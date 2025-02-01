#ifndef VM_SCRIPT_ARGUMENTS_H
#define VM_SCRIPT_ARGUMENTS_H

#include "vm/ScriptVariant.h"

class CScriptArguments {
public:
    inline CScriptArguments() : Arguments(), NumArguments(0) {}
public:
    CScriptVariant& GetArgument(u32 index) { return Arguments[index]; }
    void AppendArg(CScriptVariant& arg) { Arguments[NumArguments++] = arg; }
private:
    CScriptVariant Arguments[12];
    u32 NumArguments;
};

#endif // VM_SCRIPT_ARGUMENTS_H