#include "Stun.h"

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

static s32 YANIM_CRUSH_TOP_INTO;
static s32 YANIM_CRUSH_TOP_LOOP;
static s32 YANIM_CRUSH_TOP_LOOP_SHAKEPAD;
static s32 YANIM_CRUSH_TOP_OUTOF;

float clamp(float d, float min, float max) 
{
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

void CacheStunAnims(CSackBoyAnim* sb, CAnimBank* ab)
{
    YANIM_CRUSH_TOP_INTO = LoadAnim(ab, 21806);
    YANIM_CRUSH_TOP_LOOP = LoadAnim(ab, 21807);
    YANIM_CRUSH_TOP_LOOP_SHAKEPAD = LoadAnim(ab, 21809);
    YANIM_CRUSH_TOP_OUTOF = LoadAnim(ab, 21810);
}

void CSackBoyAnim::Stun()
{
    CRenderYellowHead* rend = GetRenderYellowHead();
    //CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_freeze", Thing, -10000.0f, -10000.0f);
}

void CSackBoyAnim::DoThawAnims()
{
    if (ThawFrame < 0) return;

    if (ThawFrame < GetNumFrames(YANIM_CRUSH_TOP_OUTOF))
        SampleAnimi(Thing, 4, YANIM_CRUSH_TOP_OUTOF, ThawFrame, false);
    
    if (ThawFrame == 0)
    {
        DebugLog("CSackBoyAnim::DoThawAnims -> Starting stun getup animations!\n");
        //CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_break_free", Thing, -10000.0f, -10000.0f);
    }
}

int CSackBoyAnim::UpdateFreezeIdleState(int last_idle)
{
    if (!Thing->GetPCreature()->Freeziness)
    {
        ShiverFrame = 0;
        return last_idle;
    }

    if (ShiverFrame % 7 == 1)
        CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_chatter", Thing, -10000.0f, -10000.0f);
    ShiverFrame++;

    return YANIM_DEATH_ICE_IDLE;
}

void CSackBoyAnim::InitIceData()
{
    WasStunned = false;
}

void CSackBoyAnim::OnLateAnimUpdate()
{
    WasStunned = Thing->GetPCreature()->State == STATE_STUNNED;
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