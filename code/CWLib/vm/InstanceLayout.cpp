#include "vm/InstanceLayout.h"

#include <Hash.h>
#include <StringUtil.h>

u32 CInstanceLayout::MakeFieldNameHash(const char* field_name)
{
    return JenkinsHash((u8*)field_name, StringLength(field_name), 0);
}

SFieldLayoutDetails* CInstanceLayout::LookupFieldByHash(u32 field_name_hash)
{
    for (SOrderedFieldName* it = FieldNameIndex.begin(); it != FieldNameIndex.end(); ++it)
    {
        if (it->FieldNameHash == field_name_hash)
            return &FieldLayoutDetailsVec[it->LayoutIdx];
    }

    return NULL;
}

SFieldLayoutDetails* CInstanceLayout::LookupField(const char* name)
{
    return LookupFieldByHash(MakeFieldNameHash(name));
}