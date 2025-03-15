#include "PartCreature.h"
#include "PartYellowHead.h"
#include "thing.h"
#include "hook.h"

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

MH_DefineFunc(PCreature_SetJetpack, 0x0007244c, TOC0, void, PCreature*, CThing* attachment, f32 length, v2 pos);
void PCreature::SetJetpack(CThing* attachment, float length, v2 pos)
{
    return PCreature_SetJetpack(this, attachment, length, pos);
}

MH_DefineFunc(PCreature_StartGunState, 0x00073718, TOC0, void, PCreature*, CThing*);
void PCreature::StartGunState(CThing* thing)
{
    PCreature_StartGunState(this, thing);
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