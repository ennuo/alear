#include "Poppet.h"
#include "hook.h"

MH_DefineFunc(CPoppet_GetBubbleSize, 0x00343dc4, TOC1, v2, CPoppet*);
v2 CPoppet::GetBubbleSize()
{
    return CPoppet_GetBubbleSize(this);
}