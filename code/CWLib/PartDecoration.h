#ifndef PART_DECORATION_H
#define PART_DECORATION_H

#include <vector.h>
#include <GuidHash.h>

#include "Part.h"

class PRenderMesh;
class CDecoration {
public:
    m44 Offset;
    m44 OldParent;
    PRenderMesh* RenderMesh;
    u32 ParentBone;
    u32 ParentTriVert;
    float BaryU;
    float BaryV;
    float DecorationAngle;
    float DecorationScale;
    u16 PlacedBy;
    bool Reversed;
    u32 PlayModeFrame;
    CGUID PlanGUID;
    s32 OnCostumePiece;
    s32 EarthDecoration;
};

class PDecorations : public CPart {
public:
    CVector<CDecoration> Decorations;
};


#endif // PART_DECORATION_H