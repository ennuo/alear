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