#include "PartCostume.h"
#include "hook.h"

MH_DefineFunc(PCostume_SetPowerup, 0x0003dde0, TOC0, void, PCostume*, RMesh*, CResourceDescriptor<RPlan>&);
MH_DefineFunc(PCostume_RemovePowerup, 0x0005b020, TOC0, void, PCostume*, CResourceDescriptor<RPlan>&);

void PCostume::SetPowerup(RMesh* mesh, CResourceDescriptor<RPlan>& original_plan)
{
    PCostume_SetPowerup(this, mesh, original_plan);
}

void PCostume::RemovePowerup(CResourceDescriptor<RPlan>& original_plan)
{
    PCostume_RemovePowerup(this, original_plan);
}
