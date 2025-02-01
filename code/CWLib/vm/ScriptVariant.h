#ifndef SCRIPT_VARIANT_H
#define SCRIPT_VARIANT_H


#include "vm/VMTypes.h"

class CScriptObject;

class CScriptVariant {
public:
    inline CScriptVariant() : MachineType(VMT_VOID), Bits(0) {}
    inline CScriptVariant(bool value) : Bool(value), MachineType(VMT_BOOL) {}
    inline CScriptVariant(s32 value) : S32(value), MachineType(VMT_S32) {}
    inline CScriptVariant(u32 value) : S32(value), MachineType(VMT_S32) {}
    inline CScriptVariant(float value) : F32(value), MachineType(VMT_F32) {}
    CScriptVariant(CScriptObject* value);
public:
    EMachineType MachineType;
    union
    {
        u32 Bits;
        bool Bool;
        wchar_t Char;
        s32 S32;
        // s64 S64;
        float F32;
        // double F64;
        const wchar_t* String;
        u32 ThingUID;
        u32 ScriptUID;
    };
    m44 M44;
    v4 V4;
};

#endif // SCRIPT_VARIANT_H