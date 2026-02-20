#ifndef EXPLODE_H
#define EXPLODE_H

#include "PlayerNumber.inl"

class CThing;

enum
{
    kExplosionFlags_None = 0,
    kExplosionFlags_DontAffectPlayer = (1 << 0),
    kExplosionFlags_DontKillPlayer = (1 << 1),
    kExplosionFlags_DisableFluidBlobs = (1 << 2),
    kExplosionFlags_DisableGlassShatter = (1 << 3),
    kExplosionFlags_DisableScorchMarks = (1 << 4)
};

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

    u8 Flags;
    u8 LethalType;
    u8 CsgType;
    u8 ExplosionStyle;

    const char* ExplosionSound;
};

// the actual function definition is really simple, but
// dont feel like filling out the RMaterial structure right now,
// so we're just going to use the function pointer.
void GetExplosionInfo(CThing* thing, ExplosionInfo& info);
const char* GetExplosionSound(CThing* thing);


void AttachExplosionHooks();

extern void (*ApplyRadialForce)(ExplosionInfo const& info);

#endif // EXPLODE_H