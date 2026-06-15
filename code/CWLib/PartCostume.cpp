#include "PartCostume.h"
#include "hook.h"

MH_DefineFunc(PCostume_SetPowerup, 0x0003dde0, TOC0, void, PCostume*, RMesh*, CResourceDescriptor<RPlan>&);
MH_DefineFunc(PCostume_RemovePowerup, 0x0005b020, TOC0, void, PCostume*, CResourceDescriptor<RPlan>&);
MH_DefineFunc(PCostume_Remove, 0x0005adcc, TOC0, void, PCostume*, ECostumePieceCategory);
MH_DefineFunc(PCostume_SetMaterial, 0x0003d218, TOC0, void, PCostume*, RGfxMaterial const* mat, CResourceDescriptor<RPlan> const& plan);
MH_DefineFunc(PCostume_GetNakedBaby, 0x0003e094, TOC0, void, PCostume*);

void PCostume::SetPowerup(RMesh* mesh, CResourceDescriptor<RPlan>& original_plan)
{
    PCostume_SetPowerup(this, mesh, original_plan);
}

void PCostume::RemovePowerup(CResourceDescriptor<RPlan>& original_plan)
{
    PCostume_RemovePowerup(this, original_plan);
}

void PCostume::Remove(ECostumePieceCategory piece)
{
    PCostume_Remove(this, piece);
}

void PCostume::SetMaterial(RGfxMaterial const* mat, CResourceDescriptor<RPlan> const& plan)
{
    PCostume_SetMaterial(this, mat, plan);
}

void PCostume::GetNakedBaby()
{
    PCostume_GetNakedBaby(this);
}