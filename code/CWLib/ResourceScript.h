#ifndef RESOURCE_SCRIPT_H
#define RESOURCE_SCRIPT_H

#include "Resource.h"
#include "MMString.h"

#include "vm/VMTypes.h"
#include "vm/InstanceLayout.h"

#include "refcount.h"
#include "vector.h"

class RScript;
class CSignature;
namespace NVirtualMachine {
    class CScriptFunctionBinding;
}

class CTypeReferenceRow {
private:
    u8 FishType;
    u8 MachineType;
    u8 DimensionCount;
    u8 ArrayBaseMachineType;
    CP<RScript> Script;
    u32 TypeNameStringIdx;
};

class RScript : public CResource {
public:
    MMString<char> ClassName;
    CP<RScript> SuperClassScript;
    ModifierBits Modifiers;
    CVector<CTypeReferenceRow> TypeReferences;
private:
    char Pad[180];
public:
    CP<RScript> UpToDateScript;
    bool ExportsFixedUp;
    bool FixedUp;
    bool Finishing;
    u32 TotalInstanceSize;
    CP<CInstanceLayout> InstanceLayout;
    CP<CInstanceLayout> SuperInstanceLayout;
public:
    bool LookupFunction(CSignature const& signature, NVirtualMachine::CScriptFunctionBinding* binding) const;
    bool IsInstanceLayoutValid();
    void ForceFixup();
    void Fixup();
};

#endif // RESOURCE_SCRIPT_H