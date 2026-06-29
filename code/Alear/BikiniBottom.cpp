#include <thing.h>
#include <PartCreature.h>
#include <DebugLog.h>


void OnSubmergeCreature(PCreature* creature)
{
    
}

extern "C" uintptr_t _on_update_creature_water_depth_hook;
void AttachFloatyFluidHooks()
{
    MH_PokeBranch(0x000a59f8, &_on_update_creature_water_depth_hook);
}

