#ifndef POPPET_UTILS_EDIT_STATE_H
#define POPPET_UTILS_EDIT_STATE_H

#include "hack_thingptr.h"

class CGlueCandidate {
public:
    CThingPtr Thing;
    CThingPtr GlueTo;
};

class CSnapCandidate {
    CThingPtr Selected;
    CThingPtr SnapTo;
    v2 EdgePos;
    u32 IndexOfSelected;
    u32 EdgeOfSnapTo;
    float Distance;
    float Overlap;
    float Depth;
    float OverlapArea;
};


#endif // POPPET_UTILS_EDIT_STATE_H