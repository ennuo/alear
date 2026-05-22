#pragma once

#include <vm/VMTypes.h>

class CScriptContext;
typedef void (*NativeFunctionWrapper)(CScriptContext*, void*, u8*);

void RegisterNativeFunction(const char* class_name, const CSignature& signature, bool is_static, NativeFunctionWrapper function);
