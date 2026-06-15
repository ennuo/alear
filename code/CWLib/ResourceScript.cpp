#include "ResourceScript.h"


MH_DefineFunc(RScript_LookupFunction, 0x00098e04, TOC0, bool, RScript const*, CSignature const&, NVirtualMachine::CScriptFunctionBinding*);
bool RScript::LookupFunction(CSignature const& signature, NVirtualMachine::CScriptFunctionBinding* binding) const
{
    return RScript_LookupFunction(this, signature, binding);
}

MH_DefineFunc(RScript_Fixup, 0x000cc04c, TOC0, void, RScript*);
void RScript::Fixup()
{
    return RScript_Fixup(this);
}

MH_DefineFunc(RScript_BlockUntilLoaded, 0x000d3f90, TOC0, bool, RScript*); 
bool RScript::BlockUntilLoaded() 
{ 
    return RScript_BlockUntilLoaded(this); 
} 

bool RScript::IsInstanceLayoutValid() const
{
    return false;
    // if (FixedUp && ExportsFixedUp)
    // {
    //     if (!SuperClassScript) return true;
    //     if (SuperClassScript->IsInstanceLayoutValid() && SuperInstanceLayout.GetRef() != NULL)
    //         return SuperClassScript->GetInstanceLayout() == SuperInstanceLayout;
    // }

    // return false;
}

bool RScript::FixupExports() const
{
    if (!ExportsFixedUp)
    {
        FixupFieldDefinitions();
        ExportsFixedUp = true;
    }

    return true;
}

extern void EnsureStaticInstanceExists(const RScript*);

bool RScript::FixupStaticInstance()
{
    if (!WantGetStaticInstance) return true;
    EnsureStaticInstanceExists(this);
    WantGetStaticInstance = false;
}

bool RScript::FixupFieldDefinitions() const
{
    u32 offset = 0, static_offset = 0;
    if (SuperClassScript)
    {
        SuperClassScript->FixupExports();
        offset = SuperClassScript->GetInstanceSize();
    }

    for (u32 i = 0; i < FieldDefinitions.size(); ++i)
    {
        CFieldDefinitionRow& field = FieldDefinitions[i];
        if (field.Modifiers.IsSet(MT_NATIVE))
        {
            field.InstanceOffset = ~0ul;
            field.FieldNameHash = 0;
        }
        else
        {
            const CTypeReferenceRow* type_ref = GetType(field.TypeReferenceIdx);
            u32 type_size = GetTypeSize(type_ref->GetMachineType());

            field.FieldNameHash = CInstanceLayout::MakeFieldNameHash(LookupStringA(field.NameStringIdx));

            if (field.Modifiers.IsSet(MT_STATIC))
            {
                static_offset = RoundUpPow2(static_offset, type_size);
                field.InstanceOffset = static_offset;
                static_offset += type_size;
            }
            else
            {
                offset = RoundUpPow2(offset, type_size);
                field.InstanceOffset = offset;
                offset += type_size;
            }
        }
    }

    TotalInstanceSize = offset;

    WantGetStaticInstance = static_offset != 0;

    return true;
}

const char* RScript::LookupStringA(u32 string_idx) const
{
    return &StringATable[StringAIndices[string_idx]];
}

void RScript::ForceFixup()
{
    FixedUp = false;
    ExportsFixedUp = false;
    InstanceLayout = NULL;
    SuperInstanceLayout = NULL;
}