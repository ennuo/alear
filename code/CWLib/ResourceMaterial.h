#ifndef RESOURCE_MATERIAL_H
#define RESOURCE_MATERIAL_H

#include <MMString.h>

#include "Resource.h"
#include "CompactCollision.h"
#include "ResourceGFXTexture.h"

class RMaterial : public CResource {
public:
    void InitializeExtraData();
    void DestroyExtraData();
public:
    CCompactMaterial CompactMaterial;
    float IDensity;
    float Traction;
    float GravityMultiplier;
    float AirResistanceMultiplier;
    u32 SoundEnum;
    float Restitution;
    bool ShiftGrip;
    bool Dissolvable;
    bool Breakable;
    float BreakMinVel;
    float BreakMinForce;
    bool Explodable;
    float ExplosionMinRadius;
    float ExplosionMaxRadius;
    float ExplosionMaxVel;
    float ExplosionMaxAngVel;
    float ExplosionMaxForce;
    bool Bullet;
    bool CuttableByExplosion;

    bool ExplosionIgnoresPlayer;
    bool DisableExplosionCSG;
    bool DisableExplosionParticles;
    CP<RTexture> ScorchMarkTexture;
    MMString<char> ExplosionSound;
};

#endif // RESOURCE_MATERIAL_H