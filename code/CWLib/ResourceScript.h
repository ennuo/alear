#ifndef RESOURCE_SCRIPT_H
#define RESOURCE_SCRIPT_H

#include "Resource.h"
#include "MMString.h"

#include "vm/VMTypes.h"

#include "refcount.h"
#include "vector.h"

class RScript;

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
    // ...
};

#endif // RESOURCE_SCRIPT_H