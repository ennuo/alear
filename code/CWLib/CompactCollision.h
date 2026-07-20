#pragma once

#include <SweepAndPrune.h>

class PShape;
class PBody;

inline v4 mergex(floatInV2 a, v4 s)
{
    return s.setX(a);
}

inline floatInV2 splatx(v4 a)
{
    return vfloat(a.getX().get128());
}

inline floatInV2 splaty(v4 a)
{
    return vfloat(a.getY().get128());
}

inline floatInV2 splatz(v4 a)
{
    return vfloat(a.getZ().get128());
}

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

class CCompactMass {
public:
    inline floatInV2 GetAngVel() const { return splatx(Scalars); }
    inline floatInV2 GetIMoment() const { return splaty(Scalars); }
    inline floatInV2 GetIMass() const { return splatz(Scalars); }
    inline v2 GetPosVel() const { return PosVel; }
    inline v2 GetPos() const { return Pos; }
    PBody* GetBody() const;
    CCompactMass* GetProxy() const;
    inline void SetAngVel(floatInV2 a) { Scalars = mergex(a, Scalars); }
    void SetIMoment(floatInV2);
    void SetIMass(floatInV2);
    
    inline void SetPosVel(v2 v)
    {
        PosVel = v;
    }

    void SetPos(v2);
    void SetBody(PBody*);
    void SetProxy(CCompactMass*);
    void SetOldPosVel(v2);
    void SetOldAngVel(floatInV2);
    v2 GetOldPosVel();
    floatInV2 GetOldAngVel();
public:
    CCompactMass();
    bool IsMovable();
private:
    v2 PosVel;
    v2 Pos;
    v4 Scalars;
};

class CCompactConvex {
public:
    enum
    {
        CIRCLE_CENTER_GLOBAL_IDX = 0,
        CIRCLE_CENTER_LOCAL_IDX = 1,
        CACHE_SIZE = 4
    };
public:
    v2 VertexCache[4];
    void* DummyForLBP1;
    CCompactMaterial* Material;
    PShape* Shape;
    const v2* SharedVerticesBase;
    u16 FirstIndex;
    u16 NumIndices;
    CCompactCollidability Collidability;
    u16 OldNumber;
    u16 NewNumber;
    u32 LastVisibleFrame;
    float MinX;
    float MinY;
    float MinZ;
    float MaxX;
    float MaxY;
    float MaxZ;
    float Restitution;
    u8 LayerFlag;
    u8 LethalType;
    bool Collidable;
    bool IsSticky;
};