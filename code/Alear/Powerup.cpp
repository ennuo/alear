#include "Powerup.h"
#include "hook.h"
#include "ppcasm.h"
#include "vector.h"
#include "mmalex.h"

#include "ResourceSystem.h"
#include "MMAudio.h"
#include "vm/NativeRegistry.h"
#include "vm/NativeFunctionCracker.h"
#include "ResourceGFXMesh.h"
#include "ResourcePlan.h"
#include "ResourceLevel.h"
#include <PartCreature.h>

#include "WinterBlast.h"


float gSackboyThickness;
CRawVector<v2, CAllocatorMMAligned128> gSackboyPolygon;
CRawVector<unsigned int> gSackboyLoops;

void LoadSackboyPolygon()
{
    CP<RLevel> level = LoadResourceByKey<RLevel>(8571, 0, STREAM_PRIORITY_DEFAULT);
    level->BlockUntilLoaded();

    CThing* sackboy = level->WorldThing->GetPWorld()->GetThingByUID(0x520);
    PShape* shape = sackboy->GetPShape();

    gSackboyThickness = shape->Thickness;
    gSackboyPolygon = shape->Polygon;
    gSackboyLoops = shape->Loops;
}

MH_DefineFunc(ForceSum, 0x00043980, TOC0, void, PShape const* shape, unsigned int n, v2& sum, float& length_sum, float min_vel_length);

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

        case STATE_FROZEN:
        {
            CRenderYellowHead* rend = thing->GetPYellowHead()->GetRenderYellowHead();
            if (rend != NULL && rend->SackBoyAnim != NULL)
                rend->SackBoyAnim->OnThaw();

            PShape* shape = thing->GetPShape();
            if (shape != NULL)
            {

                shape->Thickness = gSackboyThickness;

                CRawVector<v2, CAllocatorMMAligned128> vertices = gSackboyPolygon;
                shape->SetPolygon(vertices, gSackboyLoops);

                shape->OldMMaterial = NULL;
                CP<RMaterial> yellowhead = LoadResourceByKey<RMaterial>(8390, 0, STREAM_PRIORITY_DEFAULT);
                yellowhead->BlockUntilLoaded();
                shape->SetMaterial(yellowhead);
            }

            creature.Freeziness = 0;
            creature.WaterTimer = 0;
            
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
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(71445);
            costume->SetPowerup(mesh, desc);
            
            break;
        }
        case STATE_FROZEN:
        {
            CRenderYellowHead* rend = thing->GetPYellowHead()->GetRenderYellowHead();
            if (rend != NULL && rend->SackBoyAnim != NULL)
                rend->SackBoyAnim->OnFreeze();

            PShape* shape = thing->GetPShape();
            if (shape != NULL)
            {
                shape->Thickness = creature.Config->FrozenBoxThick;

                CRawVector<v2, CAllocatorMMAligned128> vertices = gSackboyPolygon;
                for (v2* it = vertices.begin(); it != vertices.end(); ++it)
                {
                    v4& v = *(v4*)it;
                    v -= creature.Config->FrozenBoxOffset;
                    v = Vectormath::Aos::mulPerElem(v, creature.Config->FrozenBoxScale);
                }

                shape->SetPolygon(vertices, gSackboyLoops);

                shape->OldMMaterial = shape->MMaterial;

                

                CP<RMaterial> frozenhead = LoadResourceByKey<RMaterial>(20560, 0, STREAM_PRIORITY_DEFAULT);
                frozenhead->BlockUntilLoaded();
                shape->SetMaterial(frozenhead);
            }


            break;
        }
    }
}

bool CanSwim(PCreature& creature)
{
    EState state = creature.State;
    return
        state == STATE_NORMAL_ ||
        state == STATE_GUN ||
        state == STATE_GAS_MASK;
}

bool IsPowerupState(PCreature* creature)
{
    EState state = creature->State;
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

    // If we're touching ice, check if we're touching any other lethals
    if (lethal == LETHAL_ICE)
    {
        for (int i = LETHAL_FIRE; i < LETHAL_TYPE_COUNT; ++i)
        {
            if (i == LETHAL_ICE) continue;

            CThingPtr& ptr = creature.Fork->hurt_by[i];
            if (ptr.GetThing() == NULL) continue;

            creature.LethalForce = creature.Fork->hurt_force[i];
            creature.TypeOfLethalThingTouched = i;

            return IsLethalInstaKill(creature, (ELethalType)i);
        }
    }

    // Gas mask obviously is invulnerable to gas
    if (state == STATE_GAS_MASK && lethal == LETHAL_POISON_GAS) return false;

    if (state == STATE_FROZEN)
    {
        if (lethal == LETHAL_SPIKE || lethal == LETHAL_POISON_GAS || lethal == LETHAL_ELECTRIC)
        {
            creature.TypeOfLethalThingTouched = LETHAL_ICE;
            return false;
        }

        if (lethal == LETHAL_FIRE)
        {
            v2 force = creature.Fork->hurt_force[LETHAL_FIRE];
            return force.getY() >= 0.0f;
        }
    }

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

void RemoveAbility(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;

    if (IsPowerupState(creature))
        creature->SetState(STATE_NORMAL_);
    
    if (creature->HasScubaGear)
    {
        CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_drop", thing, -10000.0f, -10000.0f);
        creature->SetScubaGear(false);
    }
}

bool CanFloat(PCreature& creature)
{
    if (creature.State == STATE_FROZEN)
    {
        return creature.Fork->AmountBodySubmerged > 0.001f || creature.Fork->AmountHeadSubmerged > creature.Config->AmountSubmergedToNotBreath;
    }
    
    return creature.Fork->IsSwimming;
}

void OnCreatureStateUpdate(PCreature& creature)
{
    if (creature.State != STATE_FROZEN)
    {
        const int MAX_FREEZINESS = creature.Config->FramesTillFreeze;
        if (creature.IsTouchingIce())
        {
            creature.Freeziness = MIN(creature.Freeziness + 1, MAX_FREEZINESS);
            if (creature.Freeziness >= MAX_FREEZINESS)
                creature.SetState(STATE_FROZEN);
        }
        else creature.Freeziness = MAX(creature.Freeziness - 1, 0);   
    }


    v2 sum; float length_sum;
    ForceSum(creature.GetThing()->GetPShape(), 0, sum, length_sum, -FLT_MAX);
    float mixed_body_sum = creature.LastForceSum + length_sum;

    if (creature.State != STATE_FROZEN)
    {
        if (creature.Fork->hurt_by[LETHAL_ICE].GetThing() != NULL)
        {
            v2 force = creature.Fork->hurt_force[LETHAL_ICE];
            v2 normal = creature.Fork->hurt_normal[LETHAL_ICE];
            
            float fx = force.getX();
            float fy = force.getY();
            float nx = normal.getX();
            float ny = normal.getY();

            if (fy >= 0.0f)
            {
                if (mixed_body_sum >= creature.Config->ForceToFreeze)
                creature.SetState(STATE_FROZEN);
                if (mixed_body_sum >= creature.Config->ForceToShatterOnFreeze)
                    creature.SetState(STATE_DEAD);
            }
            // If we're hit on the head, just freeze.
            else creature.SetState(STATE_FROZEN);
        }
    }

    creature.LastForceSum = length_sum;

    CInput* input = creature.GetInput();
    if (input == NULL) return;

    // Handle all custom powerup/sackboy states here
    switch (creature.State)
    {
        case STATE_FROZEN:
        {
            // So the player doesn't drown when frozen in ice
            // while under water.
            creature.AirTimeLeft = 900;
            creature.Freeziness = 0;

            if (creature.Fork->AmountBodySubmerged > 0.001f || creature.Fork->AmountHeadSubmerged > creature.Config->AmountSubmergedToNotBreath)
            {
                creature.StateTimer = 0;
                creature.WaterTimer += 1;

                if (creature.WaterTimer > creature.Config->FramesTillMeltInWater)
                    creature.SetState(STATE_NORMAL_);
            }
            else if (mixed_body_sum > creature.Config->ForceToSmashWhenFrozen)
                creature.SetState(STATE_DEAD);


            if (creature.Fork->hurt_by[LETHAL_FIRE].GetThing() != NULL)
            {
                v2 force = creature.Fork->hurt_force[LETHAL_FIRE];
                if (force.getY() < 0.0f)
                    creature.SetState(STATE_NORMAL_);
            }

            if (creature.StateTimer > creature.Config->FramesTillFrozenToDeath)
            {
                creature.TypeOfLethalThingTouched = LETHAL_ICE;
                creature.SetState(STATE_DEAD);
            }

            if ((input->ButtonsOld & PAD_BUTTON_TRIANGLE) == 0 && (input->Buttons & PAD_BUTTON_TRIANGLE) != 0)
                creature.SetState(STATE_NORMAL_);
            
            break;
        }
    }
}

extern "C" void _creatureupdate_hook();
extern "C" void _creatureupdate_handlecollisions_hook();
extern "C" void _animupdate_walkable_hook();
extern "C" void _updatebodyangle_walkable_hook();
extern "C" void _creature_popit_hook();

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
    MH_Poke32(0x003551fc, B(&_creature_popit_hook, 0x003551fc));

    MH_InitHook((void*)0x0040a828, (void*)&RemoveAbility);

    RegisterNativeFunction("TriggerCollectGasMask", "CollectGasMask__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGasMask>);
    RegisterNativeFunction("TriggerCollectDiverSuit", "CollectDiverSuit__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectDiverSuit>);
}