#include "RenderYellowHead.h"
#include <hook.h>

MH_DefineFunc(CRenderYellowHead_RemoveSackBoyAnim, 0x000e5c34, TOC0, void, CRenderYellowHead*);
void CRenderYellowHead::RemoveSackBoyAnim()
{
    return CRenderYellowHead_RemoveSackBoyAnim(this);
}