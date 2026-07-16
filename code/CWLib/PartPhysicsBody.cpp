#include <PartPhysicsBody.h>

v2 PBody::GetVel(v2 pos) const
{
    if (Frozen == 0)
        return GetPosVel() + ((pos - Fork->CompactMass.GetPos()).Right() * GetAngVel());
    return v2(0.0f);
}