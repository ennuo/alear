#include "NativeRegistry.h"
#include "hook.h"

MH_DefineFunc(RegisterNativeFunction_Internal, 0x00184d44, TOC0, void, const char* class_name, const CSignature& signature, bool is_static, NativeFunctionWrapper function);

void RegisterNativeFunction(const char* class_name, const CSignature& signature, bool is_static, NativeFunctionWrapper function)
{
    RegisterNativeFunction_Internal(class_name, signature, is_static, function);
}