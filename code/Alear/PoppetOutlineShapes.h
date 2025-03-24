#ifndef POPPET_OUTLINE_SHAPES_H
#define POPPET_OUTLINE_SHAPES_H

#include <GuidHash.h>
#include <vector.h>

class CPoppetOutline {
public:
    CGUID Plan;
    CGUID Mesh;
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