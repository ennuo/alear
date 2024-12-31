#include "PartYellowHead.h"
#include "hook.h"


MH_DefineFunc(PYellowHead_GetActivePosition, 0x00036598, TOC0, v4hack, const PYellowHead* const);
v4 PYellowHead::GetActivePosition() const
{
    return PYellowHead_GetActivePosition(this).V;
}

MH_DefineFunc(PYellowHead_GetColour, 0x000281f0, TOC0, c32, PYellowHead*, EPlayerColour);
c32 PYellowHead::GetColour(EPlayerColour colour)
{
    return PYellowHead_GetColour(this, colour);
}

MH_DefineFunc(PYellowHead_CollectItem, 0x00055480, TOC0, void, PYellowHead*, CThing*, bool);
void PYellowHead::CollectItem(CThing* egg_holding_thing, bool new_item)
{
    return PYellowHead_CollectItem(this, egg_holding_thing, new_item);
}

MH_DefineFunc(PYellowHead_GetSyncedProfile, 0x00027cb8, TOC0, const CP<RSyncedProfile>&, const PYellowHead*);
const CP<RSyncedProfile>& PYellowHead::GetSyncedProfile() const
{
    return PYellowHead_GetSyncedProfile(this);
}

MH_DefineFunc(PYellowHead_GetLocalProfile, 0x0002788c, TOC0, const CP<RLocalProfile>&, const PYellowHead*);
const CP<RLocalProfile>& PYellowHead::GetLocalProfile() const
{
    return PYellowHead_GetLocalProfile(this);
}

MH_DefineFunc(PYellowHead_GetInput, 0x000277b0, TOC0, CInput*, const PYellowHead*);
CInput* PYellowHead::GetInput() const
{
    return PYellowHead_GetInput(this);
}