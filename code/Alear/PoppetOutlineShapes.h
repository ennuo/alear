#ifndef POPPET_OUTLINE_SHAPES_H
#define POPPET_OUTLINE_SHAPES_H

#include <GuidHash.h>
#include <vector.h>

class CPoppetOutline {
public:
    inline bool operator<(const CPoppetOutline& rhs) const { return Plan < rhs.Plan; }
public:
    u32 Plan;
    u32 Mesh;
};

class CPoppetOutlineConfig {
public:
    inline CPoppetOutlineConfig() : Outlines() {}
public:
    CVector<CPoppetOutline> Outlines;
};

extern CPoppetOutlineConfig gPoppetOutlineData;
void LoadOutlinePolygons();

#define E_OUTLINES_KEY (3256491230u)

#endif // POPPET_OUTLINE_SHAPES_H