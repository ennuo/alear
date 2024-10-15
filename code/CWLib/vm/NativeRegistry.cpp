#include "NativeRegistry.h"
#include "hook.h"

MH_DefineFunc(RegisterNativeFunction_Internal, 0x00184d44, TOC0, void, char* class_name, CSignature& signature, bool is_static, NativeFunctionWrapper function);

void RegisterNativeFunction(char* class_name, CSignature& signature, bool is_static, NativeFunctionWrapper function)
{
    RegisterNativeFunction_Internal(class_name, signature, is_static, function);
}

void RegisterNativeFunction(char* class_name, char* function_signature, bool is_static, NativeFunctionWrapper function)
{
    CSignature signature(function_signature);
    RegisterNativeFunction_Internal(class_name, signature, is_static, function);
}