#include <Launcher.h>
#include <ResourceScript.h>
#include <ResourceGame.h>
#include <PartGroup.h>
#include <PartEffector.h>
#include <PartPhysicsWorld.h>
#include <PartCreature.h>
#include <PartSpriteLight.h>
#include <SackBoyAnim.h>
#include <thing.h>
#include <mmalex.h>

#include <PartShape.h>
#include <CollisionQuery.h>
#include <CollisionIterate.h>
#include <MMAudio.h>

const u32 E_KEY_LAUNCHER_FF = 0x12d39;

extern "C" void _creature_bouncepad_hook();

#include <DebugLog.h>

bool UpdateLauncherAnimation(CThing* launcher, int& frame)
{
    PScript* script;
    if (launcher == NULL || (script = launcher->GetPScript()) == NULL) return false;
    
    frame = MAX(frame - 1, 0);
    float factor = (frame > 7) ? 1.0 - (frame - 7) : frame / 7.0f;

    PRenderMesh* mesh = launcher->GetPRenderMesh();
    if (mesh != NULL && 1 < mesh->BoneThings.size())
    {
        m44 wpos = launcher->GetPPos()->GetWorldPosition();
        CThing* bone = mesh->BoneThings[1];
        wpos.setCol3(wpos.getCol3() + (wpos.getCol2() * (factor * 20.0f + 60.0f)));
        bone->GetPPos()->SetWorldPos(wpos, false, 0);
    }

    Launcher::SetAnimationFactor(launcher, factor);

    PShape* shape = launcher->GetPShape();
    if (shape != NULL)
        shape->EditorColourTint = v4(1.0 - factor) + (v4(2.0f, 2.0f, 2.0f, 1.0f) * factor);
    
    return frame != 0;
}

void CSackBoyAnim::DoLauncherAnims()
{
    if (JustLaunchedOff != NULL && JustLaunchedOff != LastLaunchedOff)
    {
        LastLaunchedOff = JustLaunchedOff;
        
        CAudio::PlaySample(CAudio::gLBP2SFX, "gameplay/lbp2/bouncer/bouncer", LastLaunchedOff, Launcher::GetDistance(LastLaunchedOff));
        LaunchFrame = 0;
        UpdateLauncherAnimation(LastLaunchedOff, LaunchFrame);
        LaunchFrame = 8;
    }

    JustLaunchedOff = NULL;

    if (LastLaunchedOff != NULL && !UpdateLauncherAnimation(LastLaunchedOff, LaunchFrame))
        LastLaunchedOff = NULL;

}

CThing* FindPlayerHead(CThing* thing)
{
    if (thing == NULL || thing->GetPYellowHead() == NULL) return NULL;
    CThing* child = thing->FirstChild;
    while (child != NULL)
    {
        if (child->ObjectType == OBJECT_HEAD)
            return child;
        child = child->NextSibling;
    }

    return NULL;
}

CThing* PCreature::GetTouchingBouncepad(bool ignore_layer)
{
    const u32 legs = GetNumLegs();
    if (State == STATE_JETPACK || legs == 0) return NULL;
    const float z = GetThing()->GetPPos()->GetWorldPosition().getCol3().getZ();
    for (u32 i = 0; i < legs; ++i)
    {
        if (Fork->GroundDistance > (GetLegLength(i) + GetMaxDistForFeetTouching(i))) continue;
        CThing* thing = Fork->LimbNearestObject[i].Object;
        if (thing == NULL || thing->ObjectType != OBJECT_BOUNCE_PAD_SURFACE) continue;
        PShape* shape = thing->GetPShape();
        if (shape && !ignore_layer)
        {
            if (z < (float)shape->Fork->Min.getZ()) return NULL;
            if ((float)shape->Fork->Max.getZ() < z) return NULL;
        }

        CThing* launcher = Launcher::FindFromSurface(thing);
        if (launcher != NULL)
            return launcher;
    }

    return NULL;

}

v2 Rotate(float launch_ang, v2 launch_vec)
{
    float radians = launch_ang * (3.14159 / 180) * -1.0f;
    float cos = mmalex::cos(radians);
    float sin = mmalex::sin(radians);

    return v2(
        launch_vec.getX() * cos - launch_vec.getY() * sin,
        launch_vec.getX() * sin + launch_vec.getY() * cos
    );
}

bool PCreature::DoLaunch(CThing* launcher)
{
    if (!Launcher::IsActive(launcher)) return false;
    if (!Launcher::IsAutoActivated(launcher) && !GetJumpInput())
        return false;

    v2 launch_vel = 
        Vectormath::Aos::normalize(launcher->GetPPos()->GetWorldPosition().getCol2()) * 
        Launcher::GetSpeed(launcher);
    
    launch_vel = Rotate(Launcher::GetAngle(launcher), launch_vel);

    if (Fork->JumpFrame != -1 || Fork->IsSwimming) return false;

    PBody* body;
    if ((body = GetThing()->GetBodyRoot()) == NULL)
    {
        Fork->JumpFrame = -2;
        return false;
    }

    v2 launcher_vel(0.0f);
    if (launcher->GetBodyRoot() != NULL)
        launcher_vel = launcher->GetBodyRoot()->GetVel(launcher->GetPShape()->GetPosCOM());


    v2 right = launch_vel.Normal().Right();
    v2 vel = (right * dot(launcher_vel, right)) + launch_vel + launcher_vel - (right * dot(body->GetPosVel(), right));

    body->SetPosVel(mergev2(vel, body->GetPosVel()));
    CThing* head = FindPlayerHead(GetThing());
    if ((body = head->GetBodyRoot()) != NULL)
        body->SetPosVel(mergev2(vel, body->GetPosVel()));

    PYellowHead* yellowhead;
    CRenderYellowHead* render;
    if ((yellowhead = GetThing()->GetPYellowHead()) != NULL && (render = yellowhead->GetRenderYellowHead()) != NULL && render->SackBoyAnim != NULL)
        render->SackBoyAnim->JustLaunchedOff = launcher;


    Fork->JumpFrame = -2;

    return true;
}

struct LaunchCreature
{
    LaunchCreature(CThing* thing)
    {
        Creature = thing->GetPCreature();
    }

    inline void operator()(const CCompactConvex* convex, const CCompactCollisionData& o) const
    {
        v2 force = o.Force1 + o.Force2;

        const CCompactConvex* c = o.A;
        if (convex == c) c = o.B;
        else force = -force;

        PShape* shape;
        if (c == NULL || (shape = c->Shape) == NULL) return;
        CThing* thing = shape->GetThing();
        if (thing->ObjectType != OBJECT_BOUNCE_PAD_SURFACE) return;

        if (0.0f > dot(v2(thing->GetPPos()->GetWorldPosition().getCol1()), force))
        {
            CThing* launcher = Launcher::FindFromSurface(thing);
            if (launcher != NULL)
                Creature->DoLaunch(launcher);
        }
    }

    PCreature* Creature;
};

bool HandleManualBouncepad(PCreature* creature)
{
    CThing* bouncepad = creature->GetTouchingBouncepad();
    if (bouncepad != NULL)
    {
        return creature->DoLaunch(bouncepad);
    }

    return false;
}

void HandleAutoActivatedBouncepads(CThing* thing)
{
    if (thing->ObjectType != OBJECT_SACKBOY) return;
    PShape* shape = thing->GetPShape();
    for (u32 i = 0; i < shape->GetConvexes().size(); ++i)
        IterateCollisionsOnConvex(i, shape, LaunchCreature(thing));
}

namespace Launcher
{
    PScript* GetScript(CThing* launcher)
    {
        if (launcher == NULL || launcher->ObjectType != OBJECT_BOUNCE_PAD)
            return NULL;
        return launcher->GetPScript();
    }

    CThing* FindFromSurface(CThing* thing)
    {
        if (thing == NULL || thing->ObjectType != OBJECT_BOUNCE_PAD_SURFACE)
            return NULL;

        PGroup* group;
        if (thing->GroupHead != NULL && (group = thing->GroupHead->GetPGroup()) != NULL)
        {
            for (CThing** it = group->GroupMemberList.begin(); it != group->GroupMemberList.end(); ++it)
            {
                PScript* script = (*it)->GetPScript();
                if (script && script->ScriptInstance.GetScript() && script->ScriptInstance.GetScript()->GetGUID() == E_KEY_LAUNCHER_FF)
                    return *it;
            }
        }

        return NULL;
    }

    float GetDistance(CThing* launcher)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
            return script->GetValue<float>("Distance", 0.0f);
        return 0.0f;
    }

    void SetDistance(CThing* launcher, float distance)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
        {
            CScriptArguments args;
            args.AppendArg(distance);
            script->InvokeSync("SetDistance__f", args);
        }
    }

    float GetAngle(CThing* launcher)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
            return script->GetValue<float>("Angle", 0.0f);
        return 0.0f;
    }

    void SetAngle(CThing* launcher, float angle)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
        {
            CScriptArguments args;
            args.AppendArg(angle);
            script->InvokeSync("SetAngle__f", args);
        }
    }

    float GetSpeed(CThing* launcher)
    {
        PWorld* world;
        PEffector* effector;
        PScript* script;
        float multiplier = 0.0f;

        if ((script = launcher->GetPScript()) == NULL) return 0.0f;

        float distance = script->GetValue<float>("Distance", 0.0f);
        if ((world = gGame->GetWorld()) != NULL && (effector = world->GetThing()->GetPEffector()) != NULL)
            multiplier = effector->Gravity.Length();
        
        return mmalex::sqrtf((2.0f * (distance * POPPET_GRID_SIZE)) * multiplier);
    }

    float GetAnimationFactor(CThing* launcher)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
            return script->GetValue<float>("AnimationFactor", 0.0f);
        return 0.0f;
    }

    void SetAnimationFactor(CThing* launcher, float factor)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
            script->SetValue<float>("AnimationFactor", factor);
    }

    bool IsAutoActivated(CThing* launcher)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
            return script->GetValue<bool>("AutoActivated", true);
        return true;
    }

    void SetAutoActivated(CThing* launcher, bool auto_activated)
    {
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
            return script->SetValue<bool>("AutoActivated", auto_activated);
    }

    bool IsActive(CThing* launcher)
    {
        const float deadzone = 0.25f;
        PScript* script;
        if ((script = GetScript(launcher)) != NULL)
        {
            bool is_switch_triggered = script->GetValue<bool>("IsSwitchTriggered", false);
            if (!is_switch_triggered)
                return true;

            float activation = script->GetValue<float>("SwitchActivation", 0.0f);
            return !(activation <= deadzone && -deadzone <= activation);
        }

        return false;
    }

    void Attach()
    {
        MH_PokeBranch(0x00052c5c, &_creature_bouncepad_hook);
        MH_PokeMemberHook(0x0002c778, PSpriteLight::UpdateFlicker);
    }
}