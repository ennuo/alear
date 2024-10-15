#include "powerup.h"
#include "hook.h"
#include "ppcasm.h"

#include "ResourceSystem.h"
#include "MMAudio.h"
#include "vm/NativeRegistry.h"
#include "vm/NativeFunctionCracker.h"
#include "ResourceGFXMesh.h"

class RPlan : public CResource {
public:

};

void OnStateChange(PCreature& creature, EState old_state, EState new_state)
{
    CThing* thing = creature.GetThing();
    PCostume* costume = thing->GetPCostume();

    switch (old_state)
    {
        case STATE_GAS_MASK:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_shake_try", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(71445);
                costume->RemovePowerup(desc);
            }

            break;
        }
    }

    switch (new_state)
    {
        case STATE_GAS_MASK:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "character/accessories/smelly_stuff/select", thing, -10000.0f, -10000.0f);
            CP<RMesh> mesh = LoadResourceByKey<RMesh>(71438, 0, STREAM_PRIORITY_DEFAULT);
            CResourceDescriptor<RPlan> desc(71445);
            if (BlockUntilResourceLoaded(mesh))
                costume->SetPowerup(mesh, desc);
            break;
        }
    }
}

bool CanSwim(PCreature& creature)
{
    EState state = creature.State;
    return
        state == STATE_NORMAL ||
        state == STATE_GUN ||
        state == STATE_GAS_MASK;
}

bool IsPowerupState(PCreature& creature)
{
    EState state = creature.State;
    return 
        state == STATE_JETPACK ||
        state == STATE_GUN ||
        state == STATE_DIVER_SUIT ||
        state == STATE_GAS_MASK;
}

bool IsPlayableState(PCreature& creature)
{
    EState state = creature.State;
    return
        state < STATE_STUNNED ||
        state == STATE_DIVER_SUIT || 
        state == STATE_GAS_MASK;
};

bool IsLethalInstaKill(PCreature& creature, ELethalType lethal)
{
    EState state = creature.State;

    // Gas mask obviously is invulnerable to gas
    if (state == STATE_GAS_MASK && lethal == LETHAL_POISON_GAS) return false;

    return 
        lethal == LETHAL_ELECTRIC ||
        lethal == LETHAL_SPIKE ||
        lethal == LETHAL_BULLET ||
        lethal == LETHAL_DROWNED || 
        lethal == LETHAL_POISON_GAS;
}

bool IsAffectedByFire(PCreature& creature)
{
    EState state = creature.State;
    return
        state < STATE_STUNNED ||
        state == STATE_DIVER_SUIT ||
        state == STATE_GAS_MASK;
}

void CollectDiverSuit(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_DIVER_SUIT);
}

void CollectGasMask(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_GAS_MASK);
}

void OnCreatureStateUpdate(PCreature& creature)
{
    CInput* input = creature.GetInput();
    if (input == NULL) return;

    // Handle all custom powerup/sackboy states here
    switch (creature.State)
    {
        case STATE_NORMAL:
        {
            // if ((input->ButtonsOld & PAD_BUTTON_TRIANGLE) == 0 && (input->Buttons & PAD_BUTTON_TRIANGLE) != 0)
            //     creature.SetState(STATE_GAS_MASK);
            
            break;
        }
        case STATE_GAS_MASK:
        {
            // if ((input->ButtonsOld & PAD_BUTTON_TRIANGLE) == 0 && (input->Buttons & PAD_BUTTON_TRIANGLE) != 0)
            //     creature.SetState(STATE_NORMAL);
            
            break;
        }
    }
}

extern "C" void _creatureupdate_hook();
extern "C" void _creatureupdate_handlecollisions_hook();
extern "C" void _animupdate_walkable_hook();
extern "C" void _updatebodyangle_walkable_hook();
// extern "C" void _gasmask_gas_hook();

extern "C" void _lethaltouch_hook();
extern "C" void _canswim_hook();
extern "C" void _creature_statechange_hook();
extern "C" void _creature_scubagear_equip_hook();

void AlearInitCreatureHook()
{
    MH_Poke32(0x0007ba88, B(&_creatureupdate_hook, 0x0007ba88));
    MH_Poke32(0x0007bb34, B(&_creatureupdate_handlecollisions_hook, 0x0007bb34));
    MH_Poke32(0x000feb10, B(&_animupdate_walkable_hook, 0x000feb10));
    MH_Poke32(0x000520ac, B(&_updatebodyangle_walkable_hook, 0x000520ac));

    MH_Poke32(0x00074a84, B(&_lethaltouch_hook, 0x00074a84));
    MH_Poke32(0x00064be8, B(&_canswim_hook, 0x00064be8));
    MH_Poke32(0x000719a8, B(&_creature_statechange_hook, 0x000719a8));
    MH_Poke32(0x00073b1c, B(&_creature_scubagear_equip_hook, 0x00073b1c));

    RegisterNativeFunction("TriggerCollectGasMask", "CollectGasMask__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGasMask>);
    RegisterNativeFunction("TriggerCollectDiverSuit", "CollectDiverSuit__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectDiverSuit>);
}