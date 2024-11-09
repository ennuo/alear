#include "PolyBubble.h"
#include "gfxcore.h"

#include <hook.h>

MH_DefineFunc(CPolyBubble_FindBoundaryPosition, 0x0028bb50, TOC0, bool, const CPolyBubble*, v2, v2&);

extern floatInV2 loose_accel;
extern floatInV2 loose_damp;

void CPolyBubble::SetWibbly()
{
    LastWibblyFrame = gGraphicsFrameNum;
    Accel = loose_accel;
    Damp = loose_damp;
}

bool CPolyBubble::FindBoundaryPosition(v2 direction, v2& pos_out) const
{
    return CPolyBubble_FindBoundaryPosition(this, direction, pos_out);
}