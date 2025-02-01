#ifndef COMPACT_MATERIAL_H
#define COMPACT_MATERIAL_H

class __attribute__((aligned(0x10))) CCompactMaterial {
public:
    float SlidingFriction;
    float RollingFriction;
    u32 CollideType;
    u32 CollideIgnore;
    float MaxForce;
    bool Limb;
    bool Grabbable;
};

#endif // COMPACT_MATERIAL_H