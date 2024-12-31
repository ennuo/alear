#ifndef POLY_BUBBLE_H
#define POLY_BUBBLE_H

#include <vector.h>

class CPolyBubble {
public:
    void SetWibbly();
    bool FindBoundaryPosition(v2 direction, v2& pos_out) const;
public:
    u32 SpuDone;
private:
    u32 NumTetherPoints;
    u32 NumLoopsPoint;
    CRawVector<v2, CAllocatorMMAligned128> Old;
    CRawVector<v2, CAllocatorMMAligned128> Current;
    CRawVector<v2, CAllocatorMMAligned128> Target;
    float CurrentTetherLength;
    v2 PlayerPos;
    v2 HandPos;
    bool DoThetherUpdate;
    bool HeldInHand;
    float WibblyUpdateTime;
    float TetherLooseness;
    floatInV2 Damp;
    floatInV2 Accel;
    u32 LastWibblyFrame;
    bool TetherReset;
    float Fatness;
    float MaxTetherLength;
};


#endif // POLY_BUBBLE_H