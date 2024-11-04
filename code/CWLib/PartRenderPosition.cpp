#include "PartRenderPosition.h"
#include <hook.h>

MH_DefineFunc(PPos_GetBestGameplayPosv4, 0x000364ac, TOC0, v4hack, PPos*);
v4 PPos::GetBestGameplayPosv4()
{
    return PPos_GetBestGameplayPosv4(this).V;
}

MH_DefineFunc(PPos_SetWorldPos, 0x0006ead0, TOC0, void, PPos*, m44 const&, bool, unsigned int);
void PPos::SetWorldPos(m44 const& m, bool recompute_stickers, unsigned int)
{
    return PPos_SetWorldPos(this, m, recompute_stickers, 0);
}