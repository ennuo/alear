#pragma once

class PShape;

class CCompactCollidability {
public:
    u16 CollisionBits;
    u8 CollideType;
    u8 CollideIgnore;
    void* Mass;
};

class CCompactConvex {
public:
    v2 VertexCache[4];
    void* DummyForLBP1;
    void* Material;
    PShape* Shape;
    const v2* SharedVerticesBase;
    u16 FirstIndex;
    u16 NumIndices;
    CCompactCollidability Collidability;
    u16 OldNumber; // 0x58 in deploy, 0x5c in lbp1
    u16 NewNumber; // 0x5a in deploy, 0x5e in lbp1
    u32 LastVisibleFrame;
    float MinX;
    float MinY;
    float MinZ;
    float MaxX;
    float MaxY;
    float MaxZ;
    u8 LayerFlag;
    u8 LethalType;
    bool Collidable;
    bool IsSticky;
};

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