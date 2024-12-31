#ifndef EXPLODE_H
#define EXPLODE_H

#include "PlayerNumber.inl"

class CThing;

class ExplosionInfo {
public:
    inline ExplosionInfo()
    {
        memset(this, 0, sizeof(ExplosionInfo));
        PlayerTriggerer = E_PLAYER_NUMBER_NONE;
    }
public:
    v2 Center;
    float InnerRadius;
    float OuterRadius;
    float MinZ;
    float MaxZ;
    float MaxForce;
    float MaxVel;
    float MaxAngVel;
    bool IgnoreYellowHead;
    EPlayerNumber PlayerTriggerer;
};

// the actual function definition is really simple, but
// dont feel like filling out the RMaterial structure right now,
// so we're just going to use the function pointer.
extern void (*GetExplosionInfo)(CThing* thing, ExplosionInfo& info);
extern void (*ApplyRadialForce)(ExplosionInfo const& info);

#endif // EXPLODE_H