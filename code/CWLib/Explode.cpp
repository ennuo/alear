#include "Explode.h"

#include "thing.h"
#include "PartShape.h"

#include <hook.h>

#include <cell/DebugLog.h>



// MH_DefineFunc(GetExplosionInfo, 0x00211510, TOC0, void, CThing* thing, ExplosionInfo& info);
MH_DefineFunc(ApplyRadialForce, 0x0020f630, TOC0, void, ExplosionInfo const& info);

void GetExplosionInfo(CThing* thing, ExplosionInfo& info)
{
    PShape* shape;
    PPos* pos;
    new (&info) ExplosionInfo();
    if (thing == NULL || (shape = thing->GetPShape()) == NULL || (pos = thing->GetPPos()) == NULL) return;

    CP<RMaterial>& material = shape->MMaterial;

    info.Center = pos->Fork->WorldPosition * shape->COM.getCol3();
    info.InnerRadius = material->ExplosionMinRadius;
    info.OuterRadius = material->ExplosionMaxRadius;
    info.MinZ = shape->Fork->Min.getZ();
    info.MaxZ = shape->Fork->Max.getZ();

    info.MaxForce = material->ExplosionMaxForce;
    info.MaxVel = material->ExplosionMaxVel;
    info.MaxAngVel = material->ExplosionMaxAngVel;
    info.IgnoreYellowHead = material->ExplosionIgnoresPlayer;
    info.DisableExplosionCSG = material->DisableExplosionCSG;
    info.DisableExplosionParticles = material->DisableExplosionParticles;
    if (!material->ExplosionSound.empty())
        info.ExplosionSound = material->ExplosionSound.c_str();
}
