#include "AnimBank.h"
#include <hook.h>

MH_DefineFunc(CAnimBank_InitPostResource, 0x0061466c, TOC0, void, CAnimBank*);
void CAnimBank::InitPostResource()
{
    CAnimBank_InitPostResource(this);
}