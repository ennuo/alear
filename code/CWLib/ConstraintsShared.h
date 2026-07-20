#pragma once

#include <CompactCollision.h>

struct SLocalCollisionInfo { // 54

    SLocalCollisionInfo();
    SLocalCollisionInfo(const CCompactConvex*, const CCompactConvex*);

    void FetchStage0(const CCompactConvex*, const CCompactConvex*);
    void FetchStage1();
    void FetchStage2();
    void SwapFrom(const SLocalCollisionInfo&);

    const CCompactConvex* oA;
    const CCompactConvex* oB;
    CCompactMaterial* a_material;
    CCompactMaterial* b_material;
    CCompactMass* a;
    CCompactMass* b;
    CCompactMass* main_mem_a;
    CCompactMass* main_mem_b;
};
