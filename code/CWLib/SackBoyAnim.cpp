#include "SackBoyAnim.h"

#include "thing.h"
#include "RenderYellowHead.h"
#include "FluidRender.h"

#include <hook.h>

using namespace ScriptyStuff;
extern float clamp(float d, float min, float max);

void FixMeshSpheresToNotMessUpOtherObjects(RMesh* mesh)
{
    if (mesh == NULL || !mesh->IsLoaded()) return;
    for (CImplicitEllipsoid* it = mesh->mesh.ImplicitEllipsoids.begin(); it != mesh->mesh.ImplicitEllipsoids.end(); ++it)
        it->AffectWorldOnly = 2;
}

MH_DefineFunc(CSackBoyAnim_LoadAnims, 0x000f975c, TOC0, void, CSackBoyAnim*, CAnimBank*, bool);
void CSackBoyAnim::LoadAnims(CAnimBank* ab, bool cached)
{
    CSackBoyAnim_LoadAnims(this, ab, cached);
}

CRenderYellowHead* CSackBoyAnim::GetRenderYellowHead() const
{
    return Thing->GetPYellowHead()->GetRenderYellowHead();
}

void CSackBoyAnim::SetEffectMesh(RMesh* mesh)
{
    GetRenderYellowHead()->SetEffectMesh(mesh);
}

int CSackBoyAnim::GetNumFrames(int anim)
{
    return ScriptyStuff::GetNumFrames(Thing, anim);
}

int CSackBoyAnim::DoElectricDeath()
{
    PCreature* part_creature = Thing->GetPCreature();
    PYellowHead* yellowhead = Thing->GetPYellowHead();
    int lethal_type = part_creature->GetTypeOfLethalThingTouched();

    if (WasNormal)
    {
        DeathFlip = 0;
        if (part_creature->GetForceOfLethalThingTouched().getX() < 0.0f && lethal_type == LETHAL_ELECTRIC)
            DeathFlip = 1;
    }

    int anim = YANIM_DEATH;

    if (lethal_type == LETHAL_ELECTRIC)
    {
        anim = YANIM_DEATH_ELECTRIC_LOOP;

        if (yellowhead->SuicideHoldFrameCount != 0 && WasNormal)
        {
            DeathState = 1;
            DeathFrame = 33;
        }

        if (DeathState == 0 && DeathFrame >= GetNumFrames(anim)) 
        {
            DeathFrame = 0;
            DeathState += 1;
        }

        if (DeathState == 1)
            anim = YANIM_DEATH_ELECTRIC_O;
    }

    return anim;
}

void CSackBoyAnim::Explode()
{
    PYellowHead* yellowhead = Thing->GetPYellowHead();
    CRenderYellowHead* render_yellow_head = yellowhead->GetRenderYellowHead();

    FixMeshSpheresToNotMessUpOtherObjects(ExplodeMesh);
    FixMeshSpheresToNotMessUpOtherObjects(BitsMesh);

    render_yellow_head->SnapshotCostume();
    render_yellow_head->SetMesh(NULL);
    SetEffectMesh(ExplodeMesh);

    for (int c = 0; c < 6; ++c)
        SetBlendClusterRigidity(Thing, 1, c, 0.0f);
    for (int m = 0; m < 2; ++m)
        SetSoftPhysCollision(Thing, m, true, true, true, true, true);

    FramesSinceExploded = 0;
}

// Taken from sackboyanim.ff
/*
void CSackBoyAnim::OnDeath(bool suicide, bool was_frozen)
{
    PCreature* creature = Thing->GetPCreature();
    PYellowHead* yellowhead = Thing->GetPYellowHead();
    CRenderYellowHead* render_yellow_head = yellowhead->GetRenderYellowHead();

    if(GetTypeOfLethalThingTouched(thing) == LETHAL_FIRE)
        Burnilate(was_frozen);
    if(!suicide)
        if(GetTypeOfLethalThingTouched(thing) == LETHAL_NOT)

        
    CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/death_explosion", thing, -10000.0f, -10000.0f);
    Explode()

    if(!IsScoreLocked(World))
        AlterPlayerScore(PlayerNumber, GetPlayerScore * -0.050000);
}
*/

void CSackBoyAnim::DeathSmoke(f32 size)
{
    PCreature* creature = Thing->GetPCreature();
    //creature->GetForceOfLethalThingTouched()
    //posw = size
        //CFluidRender::AddFluidBlob(v4(1.0, 1.0, 1.0, 1.0), v4(0.0, 0.0, 0.0, 0.0), 4294967295, v4(1.0, 1.0, 1.0, 1.0), v4(1.0, 0.0, 0.0, 0.0), 1, 1.0);
}

// Unfinished because I don't know where to call AddFluidBlob
void CSackBoyAnim::Steam(f32 size)
{
    PCreature* creature = Thing->GetPCreature();
    //creature->GetForceOfLethalThingTouched()
    //posw = size
        //CFluidRender::AddFluidBlob(v4(1.0, 1.0, 1.0, 1.0), v4(0.0, 0.0, 0.0, 0.0), 4294967295, v4(1.0, 1.0, 1.0, 1.0), v4(1.0, 0.0, 0.0, 0.0), 1, 1.0);
}

// Taken from sackboyanim.ff
void CSackBoyAnim::Burnilate(bool was_frozen)
{
    PCreature* creature = Thing->GetPCreature();
    PYellowHead* yellowhead = Thing->GetPYellowHead();
    CRenderYellowHead* render_yellow_head = yellowhead->GetRenderYellowHead();
    
    if(was_frozen) 
    {
        RestoreMesh(Thing);
        SetEffectMesh(NULL);
        Steam(2.5f);
    }
    else
    {
        DeathSmoke(4.0f);

        render_yellow_head->SnapshotCostume();
        render_yellow_head->SetMesh(NULL);
        SetEffectMesh(BurnMesh);
    
        for (int i = 0; i < GetClusterCount(Thing, 1); ++i)
            SetBlendClusterRigidity(Thing, 1, i, 0.000937f);
    }
}

// Taken from sackboyanim.ff
void CSackBoyAnim::Gasify()
{
    PCreature* creature = Thing->GetPCreature();
    PYellowHead* yellowhead = Thing->GetPYellowHead();
    CRenderYellowHead* render_yellow_head = yellowhead->GetRenderYellowHead();

    if(creature->StateTimer - 1 > 1)
        render_yellow_head->SetMesh(NULL);
}

// Taken from sackboyanim.ff
/*
void CSackBoyAnim::Electrify()
{

}
*/

// Taken from sackboyanim.ff
// I started to label a bit of the bytecode but this is quickly becoming too much
/* 
void CSackBoyAnim::DoBarge(bool mirror, f32 walk_vel)
{
    if(BargAnim == 0)
    BargAnim = YANIM_RUN_BARGE02_INTO
    WalkFrame = 0;
    if(YANIM_RUN_BARGE02_INTO == BargAnim)
    if(BargAnim == 0)
    barg_anim = BargAnim;
    if(WalkFrame >= (GetNumFrames(YANIM_RUN_BARGE02_INTO) - 1))

    barg_anim = YANIM_RUN_BARGE02;
    WalkFrame = 0;
    barg_anim = YANIM_RUN_BARGE02;
    BargeFrames++;
    if(!barging)
        bargy = clamp((BargeFrames / 5.0f), 0.0f, 1.0f);
        
        bargy = clamp((1.0f - (BargeOut / 5.0f)), 0.0f, 1.0f);
    BargeOut++;
    if(bargy > 0.010000)
        SampleAnimi(Thing, run_slot, barg_anim, WalkFrame, true);
    if(ArmTouching == 2) 
    {
    }
    if(mirror)
        Mirror(Thing, 1, run_slot);
    Blend(Thing, 4, 4, 1, bargy); 
}
*/

void CSackBoyAnim::DoDeathAnims()
{
    PCreature* part_creature = Thing->GetPCreature();
    PYellowHead* yellowhead = Thing->GetPYellowHead();
    CRenderYellowHead* render_yellow_head = yellowhead->GetRenderYellowHead();

    CThing* thing = Thing;
    int lethal_type = part_creature->GetTypeOfLethalThingTouched();
    int creature_state = part_creature->GetState();
    bool just_root = false;
    int anim = YANIM_DEATH;
    bool loop = false;
    int blend_in_frames = 0;
    int the_root_bone = 0;

    if (WasNormal)
    {
        DeathFlip = 0;
        //if (part_creature->GetForceOfLethalThingTouched().getX() < 0.0f && lethal_type == LETHAL_ELECTRIC)
        if (part_creature->GetForceOfLethalThingTouched().getX() < 0.0f && lethal_type == LETHAL_ELECTRIC)
            DeathFlip = 1;
    }

    switch (lethal_type)
    {
        case LETHAL_POISON_GAS:
        {
            anim = YANIM_DEATH_GAS_INTO;
            if (DeathFrame == 0)
                CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/gas_death", thing, -10000.0f, -10000.0f);
            render_yellow_head->SetDissolving(DeathFrame < 2);
            // Longer gas death animation (for fun)
            //render_yellow_head->SetDissolving(DeathFrame >= GetNumFrames(anim));
            break;
        }
        case LETHAL_ELECTRIC:
        {
            anim = DoElectricDeath();
            break;
        }
        case LETHAL_FIRE:
        {
            if (creature_state == STATE_STUNNED)
            {
                anim = YANIM_DEATH_FIRE_STUN_T_L;
                loop = true;
                IsFrozen = false;
            }
            else if (creature_state == STATE_DEAD)
            {                
                if (DeathState == 0)
                {
                    DeathFrame = 0;
                    DeathState = 1;
                    CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/fire_touch_death", thing, -10000.0f, -10000.0f);
                    IsFrozen = true;
                    VelAtFreeze = OldVel;
                }

                if (DeathFrame == 1)
                    SetExternalForce(Thing, 1, VelAtFreeze);
            }

            if (creature_state != STATE_DEAD)
                SetExternalForce(Thing, 1, v4(0.0f));

            const int FADE_FRAME = 50;
            const int FADE_LENGTH = 20;

            if (FADE_FRAME < DeathFrame)
            {
                SetSoftPhysSpringScale(Thing, 1, 
                    1.0f - clamp((float)(DeathFrame - FADE_FRAME) / (float)FADE_LENGTH, 0.0f, 1.0f));
            }

            break;
        }
        case LETHAL_CRUSH:
        {
            // I'd like to trigger the "stun" animation here
            /*
            if (creature_state == STATE_STUNNED)
            {
                anim = YANIM_DEATH_FIRE_STUN_T_L;
                loop = true;
                IsFrozen = false;
            }
            */
            render_yellow_head->SetIgnoreBodyAng(true);
            if (DeathFrame == 0)
                CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/squashed_splat", thing, -10000.0f, -10000.0f);
            break;
        }
        case LETHAL_SPIKE:
        {
            render_yellow_head->SetIgnoreBodyAng(true);
            if (DeathFrame == 0)
                CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/spiked", thing, -10000.0f, -10000.0f);
            break;
        }
        case LETHAL_DROWNED:
        {
            DeathFlip = 0;
            render_yellow_head->SetIgnoreBodyAng(true);
            anim = YANIM_SWIM_DEATH;

            if (DeathFrame == 0)
            {
                CAudio::PlaySample(CAudio::gSFX, "gameplay/water/character/drown_initiate", thing, -10000.0f, -10000.0f);
            }
            else if (DeathFrame == 51)
            {
                CAudio::PlaySample(CAudio::gSFX, "gameplay/water/character/drown", thing, -10000.0f, -10000.0f);
                Explode();
            }

            blend_in_frames = 10;

            break;
        }
        case LETHAL_BULLET:
        {
            render_yellow_head->SetIgnoreBodyAng(false);
            if (DeathFrame == 0)
                CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/plasma_death", thing, -10000.0f, -10000.0f);
            DeathFrame = 1;
            break;
        }
        case LETHAL_ICE:
        {
            anim = YANIM_DEATH_ICE_INTO;
            //IsFrozen = true;
            if (DeathFrame == 0 && creature_state != STATE_FROZEN)
                VelAtFreeze = OldVel;

            if (creature_state == STATE_DEAD)
            {
                IsFrozen = false;
                anim = YANIM_DEATH;
                render_yellow_head->SetIgnoreBodyAng(true);
                render_yellow_head->SetSoftbodySim(false);

                if (WasFrozen)
                {
                    CAudio::PlaySample(CAudio::gSFX, "gameplay/lethal/ice_break_free", thing, -10000.0f, -10000.0f);
                    DeathFrame = 0;
                }
            }
        }
    }

    if (!IsFrozen)
    {
        if (DeathFlip == 1)
        {
            SampleAnimi(thing, 0, anim, DeathFrame, loop);
            Mirror(thing, 4, 0);
        }
        else
        {
            if (blend_in_frames <= DeathFrame)
            {
                SampleAnimi(thing, 4, anim, DeathFrame, loop);
            }
            else
            {
                SampleAnimi(thing, 0, anim, DeathFrame, loop);
                Blend(thing, 4, 4, 0, (float)DeathFrame / (float)blend_in_frames);
            }
        }
    }

    WasFrozen = creature_state == STATE_FROZEN;
    DeathFrame++;
}