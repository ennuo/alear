#ifndef VM_NATIVE_REGISTRY_H
#define VM_NATIVE_REGISTRY_H

#include "mmtypes.h"

#include "VMTypes.h"
#include "ScriptContext.h"

typedef void (*NativeFunctionWrapper)(CScriptContext*, void*, u8*);

void RegisterNativeFunction(char* class_name, CSignature& signature, bool is_static, NativeFunctionWrapper function);
void RegisterNativeFunction(char* class_name, char* function_signature, bool is_static, NativeFunctionWrapper function);

#endif // VM_NATIVE_REGISTRY_H