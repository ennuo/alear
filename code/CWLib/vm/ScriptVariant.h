#ifndef SCRIPT_VARIANT_H
#define SCRIPT_VARIANT_H


#include "vm/VMTypes.h"

class CScriptVariant {
private:
    EMachineType MachineType;
    union
    {
        u32 Bits;
        bool Bool;
        wchar_t Char;
        s32 S32;
        s64 S64;
        float F32;
        double F64;
        const wchar_t* String;
        u32 ThingUID;
        u32 ScriptUID;
    };
    m44 M44;
    v4 V4;
};

#endif // SCRIPT_VARIANT_H