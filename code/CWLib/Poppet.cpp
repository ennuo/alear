#include "Poppet.h"
#include "hook.h"

MH_DefineFunc(CPoppet_GetBubbleSize, 0x00343dc4, TOC1, v2, CPoppet*);
v2 CPoppet::GetBubbleSize()
{
    return CPoppet_GetBubbleSize(this);
}

MH_DefineFunc(CPoppet_RenderHoverObject, 0x00344ab4, TOC1, void, CPoppet*, CThing*, float);
void CPoppet::RenderHoverObject(CThing* thing, float outline)
{
    CPoppet_RenderHoverObject(this, thing, outline);
}