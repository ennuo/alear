#include "WinterBlast.h"

#include <RenderYellowHead.h>
#include <ResourceSystem.h>
#include <AnimBank.h>
#include <SackBoyAnim.h>
#include <PartCreature.h>
#include <thing.h>
#include <ResourceGFXMesh.h>
#include <MMAudio.h>
#include <cell/DebugLog.h>
#include "hook.h"

using namespace ScriptyStuff;

static s32 YANIM_DEATH_ICE_FROZEN;
static s32 YANIM_DEATH_ICE_IDLE;
static s32 YANIM_DEATH_ICE_WALK;
static s32 YANIM_DEATH_ICE_LOOP;
static s32 YANIM_DEATH_ICE_OUTOF;

StaticCP<RMesh> FrozenMesh;
StaticCP<RMesh> IceCubeMesh;

float clamp(float d, float min, float max) 
{
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

void CacheIceAnims(CSackBoyAnim* sb, CAnimBank* ab)
{
    YANIM_DEATH_ICE_FROZEN = LoadAnim(ab, 18625);
    YANIM_DEATH_ICE_IDLE = LoadAnim(ab, 18587);
    YANIM_DEATH_ICE_WALK = LoadAnim(ab, 19633);
    
    int index = LoadAnim(ab, 18626);
    if (gCachedAnimLoad) sb->YANIM_DEATH_ICE_INTO = index;

    YANIM_DEATH_ICE_LOOP = LoadAnim(ab, 18627);
    YANIM_DEATH_ICE_OUTOF = LoadAnim(ab, 18616);
}

void CSackBoyAnim::FreezeSolid()
{
    CRenderYellowHead* rend = GetRenderYellowHead();
    CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_freeze", Thing, -10000.0f, -10000.0f);

    OriginalCostumeMaterial = Thing->GetPCostume()->GetCurrentMaterialPlan();

    rend->SnapshotCostume();
    rend->SetEffectMesh(IceCubeMesh);

    const int m = 1;
    int clusters = GetClusterCount(Thing, m);
    for (int i = 0; i < clusters; ++i)
    {
        SetBlendClusterRigidity(Thing, m, i, 1.0f);
        SetSoftPhysClusterEffect(Thing, m, i, 0.0f);
    }

    
    rend->SetSoftbodySim(true);
}

void CSackBoyAnim::FreezeThaw()
{
    CRenderYellowHead* rend = GetRenderYellowHead();

    RestoreMesh(Thing);
    SetEffectMesh(IceCubeMesh);

    
    rend->SetSoftbodySim(false);

    ThawFrame = 0;

    const int m = 1;
    int clusters = GetClusterCount(Thing, m);
    for (int i = 0; i < clusters; ++i)
    {
        SetBlendClusterRigidity(Thing, m, i, 0.0f);
        SetSoftPhysClusterEffect(Thing, m, i, 1.0f);
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

    const int FADE_FRAME = 150;
    const int FADE_LENGTH = 150;

    if (ThawFrame == FADE_FRAME)
    {
        DebugLog("CSackBoyAnim::DoThawAnims -> Starting fade animation for thaw animation!\n");
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

    *((CP<RMesh>*)&FrozenMesh) = LoadResourceByKey<RMesh>(19954u, 0, STREAM_PRIORITY_DEFAULT);
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