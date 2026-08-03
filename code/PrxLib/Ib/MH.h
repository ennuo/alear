#pragma once

#include <Ib/Hook.h>
#include <Ib/Memory.h>

// Legacy macros for all MH dependents

#define MH_Write(address, data, size) Ib_Write(address, data, size)
#define MH_Read(address, data, size) Ib_Read(address, data, size)

inline void MH_Poke32(u32 address, u32 value)
{
    Ib_Write(address, (void*)&value, sizeof(u32));
}


#define MH_InitHook(address, hook) Ib::Hook((uintptr_t)address, (const void*)hook)
#define MH_DefineFunc(name, address, toc, type, ...) \
static Opd _##name = { (void*)(address), (void*)(toc) }; \
type(*name)(__VA_ARGS__) = (type (*)(__VA_ARGS__))&_##name;

template <typename T> void _MH_Construct(T* t) { new (t) T(); }

#define MH_PokeBranch(address, target) Ib::PokeBranch((uintptr_t)address, (uintptr_t)target)
#define MH_PokeHook(address, function) Ib::Hook((uintptr_t)address, (const void*)&function)
#define MH_PokeMemberHook(address, function) Ib::Hook((uintptr_t)address, (const void*)GetFunctionPointer(&function))
#define MH_PokeConstructorHook(address, type) Ib::Hook((uintptr_t)address, (const void*)GetFunctionPointer(&_MH_Construct<type>))
#define MH_PokeCall(address, function) Ib::ReplaceCall((uintptr_t)address, (void*)&function)
