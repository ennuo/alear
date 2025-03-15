#ifndef INSTANCE_LAYOUT_H
#define INSTANCE_LAYOUT_H

#include <refcount.h>
#include <ReflectionVisitable.h>
#include <vector.h>

#include "vm/VMTypes.h"

struct SFieldLayoutDetails {
    char FieldName[53];
    ModifierBits Modifiers;
    EBuiltInType FishType;
    EMachineType MachineType;
    u8 DimensionCount;
    EMachineType ArrayBaseMachineType;
    u32 InstanceOffset;
};

class CInstanceLayout : public CBaseCounted, CReflectionVisitable {
struct SOrderedFieldName {
    u32 FieldNameHash;
    u32 LayoutIdx;
};
public:
    u32 MakeFieldNameHash(const char* field_name);
    SFieldLayoutDetails* LookupFieldByHash(u32 field_name_hash);
    SFieldLayoutDetails* LookupField(const char* name);
private:
    CVector<SFieldLayoutDetails> FieldLayoutDetailsVec;
    CRawVector<SOrderedFieldName> FieldNameIndex;
    u32 InstanceSize;
    CRawVector<unsigned int> ObjectHandleOffsets;
};

#endif // INSTANCE_LAYOUT_H