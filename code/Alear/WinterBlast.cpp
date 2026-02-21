#include "WinterBlast.h"

#include <RenderYellowHead.h>
#include <ResourceSystem.h>
#include <AnimBank.h>
#include <SackBoyAnim.h>
#include <PartCreature.h>
#include <PartYellowHead.h>
#include <thing.h>
#include <ResourceGFXMesh.h>
#include <MMAudio.h>
#include <cell/DebugLog.h>
#include "hook.h"

#include <Powerup.h>

using namespace ScriptyStuff;

static s32 YANIM_DEATH_ICE_FROZEN;
static s32 YANIM_DEATH_ICE_IDLE;
static s32 YANIM_DEATH_ICE_WALK;
static s32 YANIM_DEATH_ICE_INTO;
static s32 YANIM_DEATH_ICE_LOOP;
static s32 YANIM_DEATH_ICE_OUTOF;
int FADE_LENGTH = 450;

StaticCP<RMesh> IceCubeMesh;

float clamp(float d, float min, float max) 
{
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

#define CONDITIONAL_LOAD(name, guid) { int index = LoadAnim(ab, guid); if (gCachedAnimLoad) sb->name = index; }

void CacheIceAnims(CSackBoyAnim* sb, CAnimBank* ab)
{
    CONDITIONAL_LOAD(YANIM_DEATH_ICE_FROZEN, 18625);
    CONDITIONAL_LOAD(YANIM_DEATH_ICE_IDLE, 18587);
    CONDITIONAL_LOAD(YANIM_DEATH_ICE_WALK, 19633);
    CONDITIONAL_LOAD(YANIM_DEATH_ICE_INTO, 18626);
    CONDITIONAL_LOAD(YANIM_DEATH_ICE_LOOP, 18627);
    CONDITIONAL_LOAD(YANIM_DEATH_ICE_OUTOF, 18616);
}

#undef CONDITIONAL_LOAD

void CSackBoyAnim::FreezeSolid()
{
    CRenderYellowHead* rend = GetRenderYellowHead();
    CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_freeze", Thing, -10000.0f, -10000.0f);

    OriginalCostumeMaterial = Thing->GetPCostume()->GetCurrentMaterialPlan();

    rend->SnapshotCostume();
    rend->SetEffectMesh(IceCubeMesh);

    const int m0 = 0;
    int clusters0 = GetClusterCount(Thing, m0);
    for (int i = 0; i < clusters0; ++i)
    {
        SetBlendClusterRigidity(Thing, m0, i, 1.0f);
        SetSoftPhysClusterEffect(Thing, m0, i, 0.0f);
    }

    const int m1 = 1;
    int clusters1 = GetClusterCount(Thing, m1);
    for (int i = 0; i < clusters1; ++i)
    {
        SetBlendClusterRigidity(Thing, m1, i, 1.0f);
        SetSoftPhysClusterEffect(Thing, m1, i, 0.0f);
    }
    
    rend->SetSoftbodySim(true);
}

bool CanFloat(PCreature* creature)
{
    if (creature->State == STATE_FROZEN)
    {
        return creature->Fork->AmountBodySubmerged > 0.001f || creature->Fork->AmountHeadSubmerged > creature->Config->AmountSubmergedToNotBreath;
    }
    
    return creature->Fork->IsSwimming;
}

void CSackBoyAnim::FreezeThaw()
{
    CRenderYellowHead* rend = GetRenderYellowHead();

    SetEffectMesh(IceCubeMesh);
    
    rend->SetSoftbodySim(false);

    ThawFrame = 0;
    
    // Disable costume pieces when frozen/dying
    // Apply skin to the frozen mesh
    //PCostume* costume = rend->GetYellowThing()->GetPCostume();
    //costume.

    PCostume* costume = rend->GetYellowThing()->GetPCostume();
    //CP<RGfxMaterial> gfxmaterial = costume->GetCurrentMaterial();
    //CResourceDescriptor<RPlan> materialplan = costume->GetCurrentMaterialPlan();
    //costume->SetMaterial(gfxmaterial, materialplan);
    
    const int m0 = 0;
    int clusters0 = GetClusterCount(Thing, m0);
    for (int i = 0; i < clusters0; ++i)
    {
		printf("cluster %d:\n", i);
        SetBlendClusterRigidity(Thing, m0, i, 0.0f);
        SetSoftPhysCollision(Thing, m0, true, true, true, true, true);
        SetSoftPhysClusterEffect(Thing, m0, i, 1.0f);
    }

    const int m1 = 1;
    int clusters1 = GetClusterCount(Thing, m1);
    for (int i = 0; i < clusters1; ++i)
    {
        SetBlendClusterRigidity(Thing, m1, i, 0.0f);
        SetSoftPhysClusterEffect(Thing, m1, i, 1.0f);
    }
}

void CSackBoyAnim::DoThawAnims()
{
    if (ThawFrame < 0) return;

    if (ThawFrame < GetNumFrames(YANIM_DEATH_ICE_OUTOF))
        SampleAnimi(Thing, 4, YANIM_DEATH_ICE_OUTOF, ThawFrame, false);
    
    if (ThawFrame == 0)
    {
        DebugLog("CSackBoyAnim::DoThawAnims -> Starting thaw animations!\n");
        CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_break_free", Thing, -10000.0f, -10000.0f);
    }

    const int FADE_FRAME = 50;

    if (ThawFrame == FADE_FRAME)
    {
        DebugLog("CSackBoyAnim::DoThawAnims -> Starting fade animation for thaw animation!\n");
        // Check if submerged when breaking free
        if(CanFloat(Thing->GetPCreature())) { FADE_LENGTH = 250; }
        else { FADE_LENGTH = 450; }
    }

    if (ThawFrame > FADE_FRAME)
    {
        SetSoftPhysSpringScale(Thing, 1, 
            1.0f - clamp((float)(ThawFrame - FADE_FRAME) / (float)FADE_LENGTH, 0.0f, 1.0f));
    }

    ThawFrame++;

    if (ThawFrame > FADE_FRAME + FADE_LENGTH)
    {
        DebugLog("CSackBoyAnim::DoThawAnims -> Thaw animation has finished, destroying effect mesh!\n");
        SetEffectMesh(NULL);
        ThawFrame = -1;
    }
}

int CSackBoyAnim::UpdateFreezeIdleState(int last_idle)
{
    if (!Thing->GetPCreature()->Freeziness)
    {
        ShiverFrame = 0;
        return last_idle;
    }

    // Only play sounds and animation if standing on ice
    // Need to check if affected by ice here
    if(Thing->GetPCreature()->IsTouchingIce())
    {
        if (ShiverFrame % 7 == 1)
            CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_chatter", Thing, -10000.0f, -10000.0f);
        ShiverFrame++;

        // Check for walk speed
        //if(Thing->GetPCreature().) 
        //    return YANIM_DEATH_ICE_IDLE;
        return YANIM_DEATH_ICE_IDLE;
    }

    return last_idle;
}

void CSackBoyAnim::InitIceData()
{
    ThawFrame = -1;
    ShiverFrame = 0;
    WasFrozen = false;
    CrossedZero = false;

    *((CP<RMesh>*)&IceCubeMesh) = LoadResourceByKey<RMesh>(21490u, 0, STREAM_PRIORITY_DEFAULT);
}

void CSackBoyAnim::OnLateAnimUpdate()
{
    WasFrozen = Thing->GetPCreature()->State == STATE_FROZEN;
}

void CSackBoyAnim::OnFreeze()
{
    FreezeSolid();
}

void CSackBoyAnim::OnThaw()
{
    FreezeThaw();
}

bool PCreature::IsTouchingIce()
{
    return Fork->hurt_by[LETHAL_ICE].GetThing() != NULL;
}