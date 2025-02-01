#ifndef EXPLODE_H
#define EXPLODE_H

#include "PlayerNumber.inl"

class CThing;

class ExplosionInfo {
public:
    inline ExplosionInfo()
    {
        memset(this, 0, sizeof(ExplosionInfo));
        ExplosionSound = "gameplay/explosives/explode";
    }
public:
    v4 Center;
    float InnerRadius;
    float OuterRadius;
    float MinZ;
    float MaxZ;
    float MaxForce;
    float MaxVel;
    float MaxAngVel;

    bool IgnoreYellowHead;
    bool DisableExplosionCSG;
    bool DisableExplosionParticles;
    
    const char* ExplosionSound;
};

// the actual function definition is really simple, but
// dont feel like filling out the RMaterial structure right now,
// so we're just going to use the function pointer.
void GetExplosionInfo(CThing* thing, ExplosionInfo& info);
const char* GetExplosionSound(CThing* thing);

extern void (*ApplyRadialForce)(ExplosionInfo const& info);

#endif // EXPLODE_H