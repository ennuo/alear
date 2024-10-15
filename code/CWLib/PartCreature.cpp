#include "PartCreature.h"
#include "hook.h"

MH_DefineFunc(PCreature_SetState, 0x0007194c, TOC0, void, PCreature*, EState);
void PCreature::SetState(EState state)
{
    PCreature_SetState(this, state);
}


MH_DefineFunc(PCreature_GetInput, 0x000277e8, TOC0, CInput*, PCreature*);
CInput* PCreature::GetInput()
{
    return PCreature_GetInput(this);
}

MH_DefineFunc(PCreature_SetScubaGear, 0x000739e4, TOC0, void, PCreature*, bool);
void PCreature::SetScubaGear(bool active)
{
    return PCreature_SetScubaGear(this, active);
}