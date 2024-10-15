#ifndef VM_TYPES_H
#define VM_TYPES_H

#include "mmtypes.h"
#include "StringUtil.h"
#include "MMString.h"
#include "Hash.h"

struct ModifierBits {
private:
    u32 Bits;
};

enum EMachineType
{
    VMT_VOID,
    VMT_BOOL,
    VMT_CHAR,
    VMT_S32,
    VMT_F32,
    VMT_VECTOR4,
    VMT_M44,
    VMT_DEPRECATED,
    VMT_RAW_PTR,
    VMT_REF_PTR,
    VMT_SAFE_PTR,
    VMT_OBJECT_REF,
    VMT_S64,
    VMT_F64,

    NUM_MACHINE_TYPES
};

enum EBuiltInType
{
    BT_VOID,
    BT_BOOL,
    BT_CHAR,
    BT_S32,
    BT_F32,
    BT_V2,
    BT_V3,
    BT_V4,
    BT_M44,
    BT_GUID,
    BT_S64,
    BT_F64
};

enum EModifierType
{
    MT_STATIC,
    MT_NATIVE,
    MT_EPHEMERAL,
    MT_PINNED,
    MT_CONST,
    MT_PUBLIC,
    MT_PROTECTED,
    MT_PRIVATE,
    MT_PROPERTY,
    MT_ABSTRACT,
    MT_VIRTUAL,
    MT_OVERRIDE,
    MT_DIVERGENT,
    MT_EXPORT
};

u32 GetTypeSize(EMachineType machine_type);
const char* GetTypeName(EMachineType machine_type);

template <typename Type>
struct NativeTypeMangle
{
    const char* Name;
    NativeTypeMangle() { Name = "undefined"; }
};

// macro for enums
// defines NativeTypeMangle<EnumType> and SConvertScriptTypes<EnumType>


#define NATIVE_TYPE_MANGLE(type, name) \
template<> \
struct NativeTypeMangle<type> \
{ \
    const char* Name; \
    NativeTypeMangle() { Name = name; } \
};


NATIVE_TYPE_MANGLE(unsigned int, "i"); // 249
NATIVE_TYPE_MANGLE(void, "v");

template <typename Type>
struct Arg
{
    const char* Name;
    
    Arg()
    {
        Name = "abc";
    }

    const char* GetName() { return Name; }
};

struct ScriptObjectUID
{
    u32 UID;
};


class CSignature {
public:
    inline CSignature(const char* name) : MangledName()
    {
        MakeName(name);
    }

    inline void MakeName(const char* name)
    {
        int len = StringLength(name);
        MangledName.assign(name, len);
        MangledNameHash = JenkinsHash((u8*)name, len, 0);

    }
private:
    MMString<char> MangledName;
    u32 MangledNameHash;
};

#endif // VM_TYPES_H