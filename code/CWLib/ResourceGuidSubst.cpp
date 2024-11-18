#include "ResourceGuidSubst.h"
#include <hook.h>

MH_DefineFunc(RGuidSubst_Get, 0x00095ce0, TOC0, bool, const RGuidSubst*, CGUID g, CGUID& o);
bool RGuidSubst::Get(CGUID g, CGUID& o) const
{
    // this is literally just a simple binary search, but im soooooo lazy
    return RGuidSubst_Get(this, g, o);
}
