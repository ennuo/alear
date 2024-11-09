#include "Poppet.h"
#include "hook.h"

MH_DefineFunc(CPoppet_GetBubbleSize, 0x00343dc4, TOC1, v2, CPoppet*);
MH_DefineFunc(CPoppet_RenderHoverObject, 0x00344ab4, TOC1, void, CPoppet*, CThing*, float);
MH_DefineFunc(CPoppet_GetMode, 0x0033efa8, TOC1, EPoppetMode, const CPoppet*);
MH_DefineFunc(CPoppet_GetSubMode, 0x0033f22c, TOC1, EPoppetSubMode, const CPoppet*);

v2 CPoppet::GetBubbleSize()
{
    return CPoppet_GetBubbleSize(this);
}

void CPoppet::RenderHoverObject(CThing* thing, float outline)
{
    CPoppet_RenderHoverObject(this, thing, outline);
}

EPoppetMode CPoppet::GetMode() const
{
    return CPoppet_GetMode(this);
}

EPoppetSubMode CPoppet::GetSubMode() const
{
    return CPoppet_GetSubMode(this);
}