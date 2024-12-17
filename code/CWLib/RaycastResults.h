#ifndef RAYCAST_RESULTS_H
#define RAYCAST_RESULTS_H

#include "hack_thingptr.h"

class CRaycastResults {
public:
    inline v4& GetHitPoint() { return HitPoint; }
    inline void SetHitPoint(v4& v) { HitPoint = v; }
public:
    v4 HitPoint;
    v4 Normal;
    float BaryU;
    float BaryV;
    u32 TriIndex;
    CThingPtr HitThing;
    CThingPtr RefThing;
    s32 OnCostumePiece;
    u32 DecorationIdx;
    bool SwitchConnector;

    // sneaking these fields in here because padding means theres more space
    s8 HitPort;
    s8 RefPort;
};

#endif // RAYCAST_RESULTS_H