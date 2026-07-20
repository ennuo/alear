#include "PartCreature.h"
#include "PartYellowHead.h"
#include <PartPhysicsJoint.h>
#include <PartScript.h>
#include <PartShape.h>
#include <thing.h>

#include <ResourceGame.h>
#include <ResourceScript.h>

#include <mmalex.h>
#include <Launcher.h>

MH_DefineFunc(PCreature_GetJumpInput, 0x00050674, TOC0, bool, const PCreature*);
bool PCreature::GetJumpInput() const
{
    return PCreature_GetJumpInput(this);
}

u32 PCreature::GetNumLegs() const
{
    if (!CachedIsEnemy)
        return Config->Limbs - Config->Arms;
    return LegList.size();
}

float PCreature::GetLegLength(u32 i) const
{
    // todo: i dont care
    if (CachedIsEnemy) return 0.0f;

    return Config->LegLength;
}

float PCreature::GetMaxDistForFeetTouching(u32 i) const
{
    if (!CachedIsEnemy) return Config->MaxDistForFeetTouching;
    return GetLegLength(i) * 0.5f;
}

MH_DefineFunc(PCreature_SetState, 0x0007194c, TOC0, void, PCreature*, EState);
void PCreature::SetState(EState state)
{
    PCreature_SetState(this, state);
}


CInput* PCreature::GetInput()
{
    CThing* thing = GetThing();
    if (thing == NULL) return NULL;
    PYellowHead* yellowhead = thing->GetPYellowHead();
    if (yellowhead == NULL) return NULL;
    return yellowhead->GetInput();
}

bool PCreature::IsGrabbing() const
{
    if (!GrabJoint) return false;
    const PJoint* joint = GrabJoint->GetPJoint();
    return joint && joint->GetB() && joint->GetB()->GetBodyRoot();
}

MH_DefineFunc(PCreature_StartGunState, 0x00073718, TOC0, void, PCreature*, CThing*);
void PCreature::StartGunState(CThing* thing)
{
    PCreature_StartGunState(this, thing);
}

MH_DefineFunc(PCreature_StopGrabbing, 0x0006010c, TOC0, void, PCreature*, PCreature*);
void PCreature::StopGrabbing(PCreature* creature)
{
    PCreature_StopGrabbing(this, creature);
}

MH_DefineFunc(PCreature_SetScubaGear, 0x000739e4, TOC0, void, PCreature*, bool);
void PCreature::SetScubaGear(bool active)
{
    return PCreature_SetScubaGear(this, active);
}

MH_DefineFunc(PCreature_BeSlapped, 0x00073800, TOC0, void, PCreature*, CThing const*, v2 force);
void PCreature::BeSlapped(CThing const* thing, v2 force)
{
    PCreature_BeSlapped(this, thing, force);
}