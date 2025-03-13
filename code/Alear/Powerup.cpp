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
            //creature.LastFrozen = 60;
            creature.WaterTimer = 0;
            
            break;
        }

        case STATE_GRAPPLE:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/grappling_hook/drop", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(104011);
                costume->RemovePowerup(desc);
            }

            break;
        }

        case STATE_BOOTS:
        {
            CAudio::PlaySample(CAudio::gSFX, "poppet/camerazone_angle_exit", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(71445);
                costume->RemovePowerup(desc);
            }

            creature.SpeedModifier = 1.0f;
            creature.JumpModifier = 1.0f;
            creature.StrengthModifier = 1.0f;

            break;
        }

        case STATE_FORCE:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/grappling_hook/drop", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(71445);
                costume->RemovePowerup(desc);
            }

            break;
        }

        case STATE_GAUNTLETS:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/power_glove/drop", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(132790);
                costume->RemovePowerup(desc);
            }

            break;
        }

        case STATE_DIVER_SUIT:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_drop", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(71445);
                costume->RemovePowerup(desc);
            }

            break;
        }

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
        case STATE_STUNNED:
        {
            CRenderYellowHead* rend = thing->GetPYellowHead()->GetRenderYellowHead();
            if (rend != NULL && rend->SackBoyAnim != NULL)
                rend->SackBoyAnim->OnFreeze();
            break;
        }

        case STATE_FROZEN:
        {
            creature.SetScubaGear(false);
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

        case STATE_GRAPPLE:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/grappling_hook/pickup", thing, -10000.0f, -10000.0f);
            
            CP<RMesh> mesh = LoadResourceByKey<RMesh>(75854, 0, STREAM_PRIORITY_DEFAULT);
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(104011);
            costume->SetPowerup(mesh, desc);
            
            break;
        }

        case STATE_BOOTS:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "poppet/camerazone_angle_enter", thing, -10000.0f, -10000.0f);
            
            CP<RMesh> mesh = LoadResourceByKey<RMesh>(81621, 0, STREAM_PRIORITY_DEFAULT);
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(71445);
            costume->SetPowerup(mesh, desc);

            creature.SpeedModifier = 1.5f;
            creature.JumpModifier = 1.5f;
            creature.StrengthModifier = 1.5f;

            // Going to write something to call here that will
            // 1. Play equip animation
            // 2. Swap out running animations
            
            break;
        }
        
        case STATE_FORCE:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/grappling_hook/pickup", thing, -10000.0f, -10000.0f);
            
            CP<RMesh> mesh = LoadResourceByKey<RMesh>(81026, 0, STREAM_PRIORITY_DEFAULT);
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(71445);
            costume->SetPowerup(mesh, desc);
            
            break;
        }
        
        case STATE_GAUNTLETS:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/power_glove/pickup", thing, -10000.0f, -10000.0f);
            
            CP<RMesh> mesh = LoadResourceByKey<RMesh>(96530, 0, STREAM_PRIORITY_DEFAULT);
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(132790);
            costume->SetPowerup(mesh, desc);
            
            break;
        }
        
        case STATE_DIVER_SUIT:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_pickup", thing, -10000.0f, -10000.0f);
            
            CP<RMesh> mesh = LoadResourceByKey<RMesh>(71438, 0, STREAM_PRIORITY_DEFAULT);
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(71445);
            costume->SetPowerup(mesh, desc);
            
            break;
        }

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
        state == STATE_GRAPPLE ||
        state == STATE_BOOTS ||
        state == STATE_FORCE ||
        state == STATE_GAUNTLETS ||
        state == STATE_DIVER_SUIT ||
        state == STATE_GAS_MASK;
}

bool IsPlayableState(PCreature& creature)
{
    EState state = creature.State;
    return
        state < STATE_STUNNED ||
        state == STATE_GRAPPLE ||
        state == STATE_BOOTS ||
        state == STATE_FORCE ||
        state == STATE_GAUNTLETS ||
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

            v2 force = creature.Fork->hurt_force[LETHAL_FIRE];
            if (force.getY() < 0.0f)
                creature.SetState(STATE_NORMAL_);

            return IsLethalInstaKill(creature, (ELethalType)i);
        }
    }

    // If we're touching a spike plate, check if we're touching spikes
    // don't know what I'm doing with this one, just testing
    if (lethal == LETHAL_NO_STAND)
    {
        for (int i = LETHAL_SPIKE; i < LETHAL_TYPE_COUNT; ++i)
        {
            if (i == LETHAL_NO_STAND) continue;

            CThingPtr& ptr = creature.Fork->hurt_by[i];
            if (ptr.GetThing() == NULL) continue;

            creature.LethalForce = creature.Fork->hurt_force[i];
            creature.TypeOfLethalThingTouched = i;

            return IsLethalInstaKill(creature, (ELethalType)i);
        }
    }

    // Gas mask obviously is invulnerable to gas
    if (state == STATE_GAS_MASK && lethal == LETHAL_POISON_GAS) return false;
    
    // Diving suit is invulnerable to drowning
    if (state == STATE_DIVER_SUIT && lethal == LETHAL_DROWNED) return false;

    if (state == STATE_FROZEN)
    {
        if (lethal == LETHAL_SPIKE || lethal == LETHAL_POISON_GAS || lethal == LETHAL_NO_STAND || lethal == LETHAL_ELECTRIC)
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
        state == STATE_GRAPPLE ||
        state == STATE_BOOTS ||
        state == STATE_FORCE ||
        state == STATE_DIVER_SUIT ||
        state == STATE_GAS_MASK;
}

void CollectGrapple(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_GRAPPLE);
}

void CollectBoots(CThing* thing, f32 speed, f32 jump)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_BOOTS);
}

void CollectForce(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_FORCE);
}

void CollectGauntlets(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_GAUNTLETS);
}

void CollectGasMask(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_GAS_MASK);
}

void CollectDiverSuit(CThing* thing)
{
    if (thing == NULL) return;
    PCreature* creature = thing->GetPCreature();
    if (creature == NULL) return;
    creature->SetState(STATE_DIVER_SUIT);
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
        const int COLD_FREEZINESS = creature.Config->FramesTillFreezeCold;
        const int WET_FREEZINESS = creature.Config->FramesTillFreezeInWater;
        if (creature.IsTouchingIce())
        {
            creature.Freeziness = MIN(creature.Freeziness + 1, MAX_FREEZINESS);

            // Should freeze faster in water
            if (creature.Fork->IsSwimming)
            { 
                if (creature.Freeziness >= WET_FREEZINESS)
                   creature.SetState(STATE_FROZEN);
            }
            // Check frames since last frozen
            //else if (creature.LastFrozen >= 60)
            //{
            //    if (creature.Freeziness >= COLD_FREEZINESS)
            //        creature.SetState(STATE_FROZEN);
            //}
            else if (creature.Freeziness >= MAX_FREEZINESS)
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
                {
                    // Check if in water
                    if (creature.Fork->IsSwimming) 
                    {
                        creature.SetState(STATE_FROZEN);
                    }
                    else 
                    {
                        creature.SetState(STATE_DEAD);
                    }
                }
            }
            // If we're hit on the head, just freeze.
            else 
            {
                // Check if creature swimming
                if (!creature.Fork->IsSwimming)
                {   
                    creature.SetState(STATE_FROZEN);
                }
            }
        }
    }

    creature.LastForceSum = length_sum;

    CInput* input = creature.GetInput();
    if (input == NULL) return;

    // Handle all custom powerup/sackboy states here
    switch (creature.State)
    {
        //case STATE_NORMAL_:
        //{
        //    creature.LastFrozen -= 1;
        //    if (creature.LastFrozen <= 0) 
        //        creature.LastFrozen = 0;
        //}

        case STATE_FROZEN:
        {
            // So the player doesn't drown when frozen in ice
            // while under water.
            creature.AirTimeLeft = 900;
            creature.Freeziness = 0;

            if (CanFloat(creature))
            {
                creature.StateTimer = 0;
                creature.WaterTimer += 1;

                // Only melt if not touching ice while in water
                if (creature.WaterTimer > creature.Config->FramesTillMeltInWater && !creature.IsTouchingIce())
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
                // Check if creature isn't moving
                //if ( < creature.Config->ForceToShatterWhileFrozen)
                //{
                    creature.TypeOfLethalThingTouched = LETHAL_ICE;
                    creature.SetState(STATE_DEAD);
                //}
            }

            if ((input->ButtonsOld & PAD_BUTTON_TRIANGLE) == 0 && (input->Buttons & PAD_BUTTON_TRIANGLE) != 0)
                creature.SetState(STATE_NORMAL_);
            
            break;
        }

        case STATE_BOOTS:
        {
            if ((input->ButtonsOld & PAD_BUTTON_CIRCLE) == 0 && (input->Buttons & PAD_BUTTON_CIRCLE) != 0)
                creature.SetState(STATE_NORMAL_);
        }

        case STATE_FORCE:
        {
            if ((input->ButtonsOld & PAD_BUTTON_CIRCLE) == 0 && (input->Buttons & PAD_BUTTON_CIRCLE) != 0)
                creature.SetState(STATE_NORMAL_);
        }

        case STATE_DIVER_SUIT:
        {
            if ((input->ButtonsOld & PAD_BUTTON_CIRCLE) == 0 && (input->Buttons & PAD_BUTTON_CIRCLE) != 0)
                creature.SetState(STATE_NORMAL_);
        }

        case STATE_GAS_MASK:
        {
            if(!creature.Fork->hurt_by[LETHAL_POISON_GAS])
            {   
                if ((input->ButtonsOld & PAD_BUTTON_CIRCLE) == 0 && (input->Buttons & PAD_BUTTON_CIRCLE) != 0)
                    creature.SetState(STATE_NORMAL_);
            }
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

    RegisterNativeFunction("TriggerCollectGrapple", "CollectGrapple__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGrapple>);
    RegisterNativeFunction("TriggerCollectBoots", "CollectBoots__Q5Thing", true, NVirtualMachine::CNativeFunction3V<CThing*, f32, f32>::Call<CollectBoots>);
    RegisterNativeFunction("TriggerCollectForce", "CollectForce__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectForce>);
    RegisterNativeFunction("TriggerCollectGauntlets", "CollectGauntlets__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGauntlets>);
    RegisterNativeFunction("TriggerCollectGasMask", "CollectGasMask__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGasMask>);
    RegisterNativeFunction("TriggerCollectDiverSuit", "CollectDiverSuit__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectDiverSuit>);
}