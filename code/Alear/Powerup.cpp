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
#include <PartYellowHead.h>
#include <Explode.h>

#include "AlearConfig.h"
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
            creature.FramesSinceFrozen = 0;
            creature.WaterTimer = 0;
            
            // STORING THIS IN AMMO COUNT, FUCK YOU!
            creature.AmmoFillFactor = 0;

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
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/dissolve_effects/explode_appear", thing, -10000.0f, -10000.0f);
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
        
        case STATE_INVINCIBLE:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/power_glove/object_release", thing, -10000.0f, -10000.0f);

            CRenderYellowHead* rend = thing->GetPYellowHead()->GetRenderYellowHead();

            rend->SetMesh(LoadResourceByKey<RMesh>(1087, 0, STREAM_PRIORITY_DEFAULT));
            
            rend->SetDissolving(false);
            //shape->EditorColour = v4(1.0, 1.0, 1.0, 1.0);
            //shape->EditorColourTint = v4(1.0, 1.0, 1.0, 1.0);
            // Set character lethal type to none
            // shape->LethalType = LETHAL_NOT;

            break;
        }

        case STATE_DIVER_SUIT:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_drop", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(2000005);
                costume->RemovePowerup(desc);
            }

            break;
        }

        case STATE_SWIMMING_FINS:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_drop", thing, -10000.0f, -10000.0f);
            if (costume != NULL)
            {
                CResourceDescriptor<RPlan> desc(2000002);
                costume->RemovePowerup(desc);
            }
            
            creature.SpeedModifier = 1.0f;
            creature.JumpModifier = 1.0f;
            creature.StrengthModifier = 1.0f;

            break;
        }
        
        case STATE_MINI_SUIT:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/dissolve_effects/expand", thing, -10000.0f, -10000.0f);

            // Set creature MeshScale to 1.0f

            break;
        }
    }

    switch (new_state)
    {
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

            // Going to write something to call here that will
            // 1. Play equip animation
            // 2. Swap out running animations
            
            break;
        }
        
        case STATE_FORCE:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/dissolve_effects/explode_appear", thing, -10000.0f, -10000.0f);
            
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

        case STATE_INVINCIBLE:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/power_glove/object_engage", thing, -10000.0f, -10000.0f);

            CRenderYellowHead* rend = thing->GetPYellowHead()->GetRenderYellowHead();

            rend->SnapshotCostume();
            rend->SetMesh(LoadResourceByKey<RMesh>(2000004, 0, STREAM_PRIORITY_DEFAULT));
            rend->SetDissolving(true);

            break;
        }

        case STATE_DIVER_SUIT:
        {
            creature.SetScubaGear(false);
            CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_pickup", thing, -10000.0f, -10000.0f);
            
            CP<RMesh> mesh = LoadResourceByKey<RMesh>(2000006, 0, STREAM_PRIORITY_DEFAULT);
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(2000005);
            costume->SetPowerup(mesh, desc);
            
            break;
        }
        
        case STATE_SWIMMING_FINS:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_pickup", thing, -10000.0f, -10000.0f);
            
            //*((CP<RMesh>*)&SwimmingFinsMesh) = LoadResourceByKey<RMesh>(2000001, 0, STREAM_PRIORITY_DEFAULT);

            CP<RMesh> mesh = LoadResourceByKey<RMesh>(2000001, 0, STREAM_PRIORITY_DEFAULT);
            mesh->BlockUntilLoaded();
            
            CResourceDescriptor<RPlan> desc(2000002);
            costume->SetPowerup(mesh, desc);
            
            break;
        }

        case STATE_MINI_SUIT:
        {
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/dissolve_effects/shrink", thing, -10000.0f, -10000.0f);

            // Set creature MeshScale to 0.5f

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
        state == STATE_GAS_MASK ||
        state == STATE_INVINCIBLE ||
        state == STATE_SWIMMING_FINS ||
        state == STATE_MINI_SUIT;
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
        state == STATE_GAS_MASK ||
        state == STATE_INVINCIBLE ||
        state == STATE_DIVER_SUIT ||
        state == STATE_MINI_SUIT;
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
        state == STATE_GAS_MASK ||
        state == STATE_INVINCIBLE ||
        state == STATE_SWIMMING_FINS ||
        state == STATE_DIVER_SUIT ||
        state == STATE_MINI_SUIT;
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
            {
                if(state == STATE_FROZEN && state != STATE_INVINCIBLE)
                {
                    creature.SetState(STATE_NORMAL_);
                }
                else
                {
                    creature.SetState(STATE_STUNNED);
                    // Set frames since frozen to be less here
                    // Reset to normal state..
                }
            }

            return IsLethalInstaKill(creature, (ELethalType)i);
        }
    }

    // If we're touching a spike plate, check if we're touching spikes
    // don't know what I'm doing with this one, just testing
    if (lethal == LETHAL_NO_STAND)
    {
        for (int i = LETHAL_FIRE; i < LETHAL_TYPE_COUNT; ++i)
        {
            if (i == LETHAL_NO_STAND) continue;

            CThingPtr& ptr = creature.Fork->hurt_by[i];
            if (ptr.GetThing() == NULL) continue;

            creature.LethalForce = creature.Fork->hurt_force[i];
            creature.TypeOfLethalThingTouched = i;

            v2 force = creature.Fork->hurt_force[LETHAL_SPIKE];
            if (force.getY() > 0.0f)
            {
                return false;
            }

            return IsLethalInstaKill(creature, (ELethalType)i);
        }
    }

    // Gas mask obviously is invulnerable to gas
    if (state == STATE_GAS_MASK && lethal == LETHAL_POISON_GAS) return false;

    // Invincible is not invulnerable to gas
    if (state == STATE_INVINCIBLE && lethal != LETHAL_POISON_GAS) return false;
    
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
            return force.getY() < 0.0f;
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
        state == STATE_GAUNTLETS ||
        state == STATE_GAS_MASK ||
        state == STATE_SWIMMING_FINS ||
        state == STATE_DIVER_SUIT ||
        state == STATE_MINI_SUIT;
}

bool IsAffectedByIce(PCreature& creature)
{
    EState state = creature.State;
    return
        state < STATE_STUNNED ||
        state == STATE_GRAPPLE ||
        state == STATE_BOOTS ||
        state == STATE_FORCE ||
        state == STATE_GAUNTLETS ||
        state == STATE_GAS_MASK ||
        state == STATE_SWIMMING_FINS ||
        state == STATE_DIVER_SUIT ||
        state == STATE_MINI_SUIT;
}

/*

bool IsPlayerSubmerged(PCreature& part_creature)
{
    float submerged;

    if(part_creature.Config->IsLoaded())
        submerged = part_creature.Config->AmountSubmergedToLosePowerups;
    else
        submerged = 0.3f;
    if(part_creature.Fork->AmountBodySubmerged <= submerged)
        return true;
    return false;
}
*/

/*
bool CheckPowerup(CThing& player, PCreature& part_creature)
{
    if (player == NULL || (part_creature = player.GetPCreature()) == NULL) return true;
    if (!IsPlayableState(part_creature)) return true;
    return false;
}
*/

void CollectGrapple(CThing* player)
{
    // Fields to add:
    // can be dropped
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;
    // Return if submerged
    //if(IsPlayerSubmerged(*part_creature)) return;
    
    float submerged;

    if(part_creature->Config->IsLoaded())
        submerged = part_creature->Config->AmountSubmergedToLosePowerups;
    else
        submerged = 0.3f;
    if(part_creature->Fork->AmountBodySubmerged > submerged) return;

    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_GRAPPLE);
}

void CollectBoots(CThing* player, f32 speed, f32 jump, f32 strength)
{
    // Fields to add:
    // can be dropped
    PCreature* part_creature;
    PScript* part_script;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;
    // Return if submerged
    //if(IsPlayerSubmerged(*part_creature)) return;

    float submerged;

    if(part_creature->Config->IsLoaded())
        submerged = part_creature->Config->AmountSubmergedToLosePowerups;
    else
        submerged = 0.3f;
    if(part_creature->Fork->AmountBodySubmerged > submerged) return;

    // I don't know what I fucked up here this time, but this crashes
    // I had it working at one point too
    //part_creature->SpeedModifier = part_script->GetValue<float>("SpeedModifier", 1.25f);
    //part_creature->JumpModifier = part_script->GetValue<float>("JumpModifier", 1.25f);
    //part_creature->StrengthModifier = part_script->GetValue<float>("StrengthModifier", 1.0f);

    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_BOOTS);
}

void CollectForce(CThing* player)
{
    // Fields to add:
    // can be dropped
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;
    // Return if submerged
    //if(IsPlayerSubmerged(*part_creature)) return;
    
    float submerged;

    if(part_creature->Config->IsLoaded())
        submerged = part_creature->Config->AmountSubmergedToLosePowerups;
    else
        submerged = 0.3f;
    if(part_creature->Fork->AmountBodySubmerged > submerged) return;
    
    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_FORCE);
}

void CollectGauntlets(CThing* player)
{
    // Fields to add:
    // can be dropped
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;
    // Return if submerged
    //if(IsPlayerSubmerged(*part_creature)) return;

    float submerged;

    if(part_creature->Config->IsLoaded())
        submerged = part_creature->Config->AmountSubmergedToLosePowerups;
    else
        submerged = 0.3f;
    if(part_creature->Fork->AmountBodySubmerged > submerged) return;

    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_GAUNTLETS);
}

void CollectGasMask(CThing* player)
{
    // Fields to add:
    // can be dropped
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;

    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_GAS_MASK);
}

void CollectSwimmingFins(CThing* player)
{
    // Fields to add:
    // can be dropped
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;

    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_SWIMMING_FINS);
}

void CollectDiverSuit(CThing* player)
{
    // Fields to add:
    // can be dropped
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;

    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_DIVER_SUIT);
}

void CollectMiniSuit(CThing* player)
{
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;

    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_MINI_SUIT);
}

void CollectInvincible(CThing* player)
{
    // Fields to add:
    // time (limit on state)
    // can be dropped
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;
    part_creature->ReactToLethal = false;
    part_creature->CanDropPowerup = true;
    part_creature->SetState(STATE_INVINCIBLE);
}

void SetJetpackTether(CThing* player, CThing* attachment, float length, v2 pos)
{
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;
    player->GetPYellowHead()->SetJetpack(attachment, length, pos);
}

/*
void StartGunState(CThing* creature_thing)
{
    PCreature* part_creature;
    PScript* part_script;
    CScriptVariant result;
    
    //int set_count = part_script->GetValue("BulletCount", result);
    int bullet_count = part_creature->Fork->BulletCount;
    int max_bullet_count = part_creature->MaxBulletCount;
    // Check if in gun state so we can refill if needed
    if(part_creature->State == STATE_GUN)
    {
        // If the ammo is empty or if the max count is 0
        if(bullet_count == 0 || max_bullet_count == 0) return;
        // If the current ammo is less than setting ammo
        //else if(bullet_count <= set_count) return;
        //deploy leftover
        //part_creature->SetState(creature_thing, STATE_NORMAL_);
    }
    CAudio::PlaySample(CAudio::gSFX, "gameplay/paintball_gun/pickup", creature_thing, -10000.0f, -10000.0f);

    //part_creature->Fork->BulletCount = set_count;
    //part_creature->MaxBulletCount = set_count; 

    part_creature->SetState(STATE_GUN);
}
*/

void CollectGun(CThing* player, CThing* creature_thing)
{
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;
    // Return if submerged
    //if(IsPlayerSubmerged(*part_creature)) return;
    float submerged;    
    if(part_creature->Config->IsLoaded())
        submerged = part_creature->Config->AmountSubmergedToLosePowerups;
    else
        submerged = 0.3f;
    if(part_creature->Fork->AmountBodySubmerged > submerged) return;
    part_creature->StartGunState(creature_thing);
}

void SetScubaGear(CThing* player, bool active)
{
    PCreature* part_creature;
    // Return if invalid
    //if(CheckPowerup(*player, *part_creature)) return;
    if (player == NULL || (part_creature = player->GetPCreature()) == NULL) return;
    if (!IsPlayableState(*part_creature)) return;

    if (active && !part_creature->HasScubaGear)
        CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_pickup", player, -10000.0f, -10000.0f);
        part_creature->SetScubaGear(active);
}

void RemoveAbility(CThing* player)
{
    if (player == NULL) return;
    PCreature* part_creature = player->GetPCreature();
    if (part_creature == NULL) return;

    if (IsPowerupState(part_creature))
        part_creature->SetState(STATE_NORMAL_);
    
    if (part_creature->HasScubaGear)
    {
        CAudio::PlaySample(CAudio::gSFX, "gameplay/water/special/aqualung_drop", player, -10000.0f, -10000.0f);
        part_creature->SetScubaGear(false);
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
    if (IsAffectedByIce(creature))
    {
        const int MAX_FREEZINESS = creature.Config->FramesTillFreeze;
        const int COLD_FREEZINESS = creature.Config->FramesTillFreezeCold;
        const int WET_FREEZINESS = creature.Config->FramesTillFreezeInWater;
        if (creature.IsTouchingIce())
        {
            creature.Freeziness = MIN(creature.Freeziness + 1, MAX_FREEZINESS);

            // Should freeze faster in water and with jetpack
            if (creature.Fork->IsSwimming || creature.State == STATE_JETPACK)
            { 
                if (creature.Freeziness >= WET_FREEZINESS)
                   creature.SetState(STATE_FROZEN);
            }
            // Check frames since last frozen
            // Should be assigned to something in the creature config later
            // Also assigned a better variable than "StateTimer" so it doesn't check for removing powerups
            else if (creature.FramesSinceFrozen < 60)
            {
                if (creature.Freeziness >= COLD_FREEZINESS)
                    creature.SetState(STATE_FROZEN);
            }
            else if (creature.Freeziness >= MAX_FREEZINESS)
                creature.SetState(STATE_FROZEN);
        }
        else creature.Freeziness = MAX(creature.Freeziness - 1, 0);   
    }


    v2 sum; float length_sum;
    ForceSum(creature.GetThing()->GetPShape(), 0, sum, length_sum, -FLT_MAX);
    float mixed_body_sum = creature.LastForceSum + length_sum;

    if (IsAffectedByIce(creature))
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
                    // DEPRECATED


                    // We don't want it to shatter in water as there should be dampening
                    //if (creature.Fork->IsSwimming) 
                    //{
                        creature.SetState(STATE_FROZEN);
                    //}
                    //else 
                    //{
                    //    creature.SetState(STATE_DEAD);
                    //}
                }
            }
            // If we're hit on the head, just freeze.
            // This should check the normal of the object instead of just head
            // since it seems to be buggy (hitting a wall triggers for example)
            else 
            {
                // Check if creature swimming
                if (!creature.Fork->IsSwimming)
                {   
                    if (mixed_body_sum >= 10000.0f)
                        creature.SetState(STATE_FROZEN);
                }
            }
        }
    }

    creature.LastForceSum = length_sum;

    CInput* input = creature.GetInput();
    if (input == NULL) return;

    CThing* thing = creature.GetThing();
    // Handle all custom powerup/sackboy states here
    
    //part_creature->CanDropPowerup = true;
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

            // TEMPORARILY DISABLED FUCK YOU
            //if (creature.StateTimer > creature.Config->FramesTillFrozenToDeath)
            //{
                // Check if creature isn't moving
                //if ( < creature.Config->ForceToShatterWhileFrozen)
                //{
                    //creature.TypeOfLethalThingTouched = LETHAL_ICE;
                    //creature.SetState(STATE_DEAD);
                //}
            //}

            //if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_BREAK_FREE_RIGHTSTICK, L"BP_BREAK_FREE"))
            //    creature.SetState(STATE_NORMAL_);
            
            // STORING THIS IN AMMO BECAUSE WHY NOT FUCK YOU
            if (creature.AmmoFillFactor >= 3)
                creature.SetState(STATE_NORMAL_);
                
            if (!gUseIceAccessibility)
            {
                if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_BREAK_FREE_TRIANGLE, L"BP_BREAK_FREE"))
                {
                    CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_shake_tinkle", thing, -10000.0f, -10000.0f);
                    creature.AmmoFillFactor++;
                }
            }
            else
            {
                if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_BREAK_FREE_SHAKE, L"BP_BREAK_FREE"))
                    creature.SetState(STATE_NORMAL_);
            }
            //if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_BREAK_FREE_CROSS, L"BP_BREAK_FREE"))
            //    creature.SetState(STATE_NORMAL_);
            //if (ShiverFrame % 3 == 1)
            //    creature.SetState(STATE_NORMAL_);

            //if ((input->ButtonsOld & PAD_BUTTON_TRIANGLE) == 0 && (input->Buttons & PAD_BUTTON_TRIANGLE) != 0)
            //    creature.SetState(STATE_NORMAL_);
            
            break;
        }
        
        case STATE_GUN:
        {
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_GUN, L"BP_REMOVE_PAINTINATOR"))
            {}
            //    creature.SetState(STATE_NORMAL_);
                
            break;
        }

        case STATE_BOOTS:
        {   
            float submerged;

            if(creature.Config->IsLoaded())
                submerged = creature.Config->AmountSubmergedToLosePowerups;
            else
                submerged = 0.3f;
            if(creature.Fork->AmountBodySubmerged > submerged)
                creature.SetState(STATE_NORMAL_);

            //if(DropPowerup(STATE_BOOTS))
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_BOOTS, L"BP_REMOVE_BOOTS"))
                creature.SetState(STATE_NORMAL_);
                
            break;
        }

        case STATE_FORCE:
        {
            float submerged;

            if(creature.Config->IsLoaded())
                submerged = creature.Config->AmountSubmergedToLosePowerups;
            else
                submerged = 0.3f;
            if(creature.Fork->AmountBodySubmerged > submerged)
                creature.SetState(STATE_NORMAL_);

            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_FORCE_BLAST, L"BP_FORCE"))
            {
                ExplosionInfo info;
                GetExplosionInfo(thing, info);
                info.Center = info.Center + v4(0.0f, -50.0f, 0.0f, 0.0f);
                info.IgnoreYellowHead = true;

                info.OuterRadius = 250.0f;
                info.InnerRadius = 250.0f;
                info.MaxVel = 100.0f;
                info.MaxForce = 1500.0f;
                info.MaxAngVel = 1.0f;

                CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/suicide_breath_relief", thing, -10000.0f, -10000.0f);
                ApplyRadialForce(info);
            }
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_FORCE, L"BP_REMOVE_FORCE"))
                creature.SetState(STATE_NORMAL_);
                
            break;
        }

        case STATE_GRAPPLE:
        {   
            float submerged;

            if(creature.Config->IsLoaded())
                submerged = creature.Config->AmountSubmergedToLosePowerups;
            else
                submerged = 0.3f;
            if(creature.Fork->AmountBodySubmerged > submerged)
                creature.SetState(STATE_NORMAL_);
                
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_GRAPPLE, L"BP_GRAPPLE")) {}
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_GRAPPLE, L"BP_REMOVE_GRAPPLE"))
                creature.SetState(STATE_NORMAL_);
                
            break;
        }
        
        case STATE_GAUNTLETS:
        {   
            float submerged;

            if(creature.Config->IsLoaded())
                submerged = creature.Config->AmountSubmergedToLosePowerups;
            else
                submerged = 0.3f;
            if(creature.Fork->AmountBodySubmerged > submerged)
                creature.SetState(STATE_NORMAL_);

            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_POWER_GLOVES, L"BP_REMOVE_POWER_GLOVES"))
                creature.SetState(STATE_NORMAL_);
                
            break;
        }

        case STATE_INVINCIBLE:
        {
            //CAudio::PlaySample(CAudio::gSFX, "gameplay/lbp2/power_glove/KO2_object_powered_up", creature.GetThing(), -10000.0f, -10000.0f);
            
            PShape* shape = creature.GetThing()->GetPShape();
            if (shape != NULL)
            {
                // Cycle editor colour here with hue shift
                
                CRenderYellowHead* rend = thing->GetPYellowHead()->GetRenderYellowHead();
                //rend->EffectMesh->EditorColour = v4();
            }

            //if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_INVINCIBLE, (const wchar_t*)NULL))
            //    creature.SetState(STATE_NORMAL_);
            // Invincibility timeout
            //if (creature.StateTimer > 120)
            //    creature.SetState(STATE_NORMAL_);
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_GRAPPLE, L"BP_REMOVE_INVINCIBLE"))
                creature.SetState(STATE_NORMAL_);
                
            break;
        }
        
        case STATE_GAS_MASK:
        {
            if(!creature.Fork->hurt_by[LETHAL_POISON_GAS])
            {   
                if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_GAS_MASK, L"BP_REMOVE_GAS_MASK"))
                    creature.SetState(STATE_NORMAL_);
            }
            
            break;
        }

        case STATE_SWIMMING_FINS:
        {
            float submerged;

            if(creature.Config->IsLoaded())
                submerged = creature.Config->AmountSubmergedToLosePowerups;
            else
                submerged = 0.3f;
                
            // Only set speed up if in water
            //if(IsPlayerSubmerged(creature))
            if(creature.Fork->AmountBodySubmerged > submerged)
            {
                creature.SpeedModifier = 50.0f;
                creature.JumpModifier = 1.0f;
                creature.StrengthModifier = 1.0f;
            }
            else
            {
                creature.SpeedModifier = 1.0f;
                creature.JumpModifier = 1.0f;
                creature.StrengthModifier = 1.0f;
            }

            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_SWIMMING_FINS, L"BP_REMOVE_SWIMMING_FINS"))
                creature.SetState(STATE_NORMAL_);
                
            break;
        }

        case STATE_DIVER_SUIT:
        {
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_DIVER_SUIT, L"BP_REMOVE_DIVER_SUIT"))
                creature.SetState(STATE_NORMAL_);
                
            break;
        }
        
        case STATE_MINI_SUIT:
        {
            if (thing != NULL && input->IsJustClicked(BUTTON_CONFIG_REMOVE_MINI_SUIT, L"BP_REMOVE_MINI_SUIT"))
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

    RegisterNativeFunction("TriggerCollectGrapple", "CollectGrapple__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGrapple>);
    RegisterNativeFunction("TriggerCollectBoots", "CollectBoots__Q5Thingfff", true, NVirtualMachine::CNativeFunction4V<CThing*, f32, f32, f32>::Call<CollectBoots>);
    RegisterNativeFunction("TriggerCollectForce", "CollectForce__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectForce>);
    RegisterNativeFunction("TriggerCollectGauntlets", "CollectGauntlets__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGauntlets>);
    RegisterNativeFunction("TriggerCollectGasMask", "CollectGasMask__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectGasMask>);
    RegisterNativeFunction("TriggerCollectSwimmingFins", "CollectSwimmingFins__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectSwimmingFins>);
    RegisterNativeFunction("TriggerCollectDiverSuit", "CollectDiverSuit__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectDiverSuit>);
    RegisterNativeFunction("TriggerCollectMiniSuit", "CollectMiniSuit__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectMiniSuit>);
    RegisterNativeFunction("TriggerCollectInvincible", "CollectInvincible__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<CollectInvincible>);
}