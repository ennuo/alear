#include "Explode.h"
#include <hook.h>

MH_DefineFunc(GetExplosionInfo, 0x00211510, TOC0, void, CThing* thing, ExplosionInfo& info);
MH_DefineFunc(ApplyRadialForce, 0x0020f630, TOC0, void, ExplosionInfo const& info);