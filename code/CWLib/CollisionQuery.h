#pragma once

#include <CompactCollision.h>

class CCompactCollisionData {
public:
    CCompactConvex* A;
    CCompactConvex* B;
    u16 AEdge;
    u16 BEdge;
    float ForceRolling;
    v2 Force1;
    v2 Force2;
    float OldVel1;
    float OldVel2;
};

struct KdTree {
    u32 Size;
    u32 MaxSize;
    void* Nodes;
    u16 NullNode;
};

class CCollisionQuery {
public:
    void* Boxes;
    void** ConvexForNumber;
    KdTree Tree;
    u32 CollisionPairCount;
    u32* SortedConvexCollisionPairs;
    u32* SwappedSortedConvexCollisionPairs;
    u32* SwappedSortedConvexCollisionPairsIndex;
    CCompactCollisionData* Collisions;
    // ...
};