#include "PartRenderPosition.h"
#include <hook.h>

MH_DefineFunc(PPos_GetBestGameplayPosv4, 0x000364ac, TOC0, v4hack, PPos*);
v4 PPos::GetBestGameplayPosv4()
{
    return PPos_GetBestGameplayPosv4(this).V;
}