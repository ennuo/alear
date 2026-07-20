#pragma once

#include <CompactCollision.h>
#include <ConstraintsShared.h>
#include <vector.h>

class PShape;
class PJoint;

class CCompactConstraint {
public:
    v2 GetAConL() const;
    v2 GetBConL() const;
    void SetAConL(v2);
    void SetBConL(v2);
    u32 GetAIndex() const;
    u32 GetBIndex() const;
    CCompactMass* GetA(CCompactMass*) const;
    CCompactMass* GetB(CCompactMass*) const;
    const CCompactMass* GetA(const CCompactMass*) const;
    const CCompactMass* GetB(const CCompactMass*) const;
    void SetA(u32);
    void SetB(u32);
    void SetForcePointer(v2*);
    v2* GetForcePointer() const;
    void SetAAConL(u32, v2);
    void SetBBConL(u32, v2);
    CCompactConstraint();
    void ApplyAccumulatedImpulse(v2, CCompactMass*) const;
private:
    v2 AConLA;
    v2 BConLB;
};

class CCompactContact : public CCompactConstraint { // 95
private:
    v4 Scalars2;
    v4 Scalars;
    v4 Scalars3;
public:
    floatInV2 GetIntersectionFix() const;
    floatInV2 GetFriction() const;
    floatInV2 GetDivNormal() const;
    floatInV2 GetDivTangential() const;
    v2 GetNormal() const;
    floatInV2 GetBoredomThreshold() const;
    floatInV2 GetMaxForce() const;
    void SetNormalBoredomMaxForce(v2, floatInV2, floatInV2);
    void SetBounceBackPtrsFlags(floatInV2, PShape*, PShape*, intInV2);
    void SetBounce(float);
    floatInV2 GetBounce() const;
    void SetBackptrs(PShape*, PShape*);
    PShape* GetBackptrA();
    PShape* GetBackptrB();
public:
    CCompactContact();
    CCompactContact(SLocalCollisionInfo&, const CCompactConvex*, const CCompactConvex*, v2*, v2, v2, v2, CCompactMass*, bool);
public:
    intInV2 Resolve(v2&, CCompactMass*) const;
    intInV2 ResolveOld(v2&, CCompactMass*) const;
    void ResolveSOA(v2*, CCompactMass*);
    floatInV2 PositionalError(CCompactMass*) const;
};


class CCompactConstraints { // 499
public:
    void ReadJoint(PJoint* const* joint_list, unsigned int joint_count, float frames);
public:
    CRawVector<CCompactMass, CAllocatorMMAligned128> VecCCompactMass;
};