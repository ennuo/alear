#ifndef PART_EFFECTOR_H
#define PART_EFFECTOR_H

#include "Part.h"

class PEffector : public CPart {
public:
    v2 PosVel;
    v2 Gravity;
    union
    {
        // these fields dont get used
        // so just going to store wind fields in them
        struct
        {
            float AngVel;
            float Viscosity;
        };
        struct
        {
            float WindDirection;
            float WindStrength;
        };
    };
    float ViscosityCheap;
    float Density;
    float ModScale;
    bool PushBack;
    bool Swimmable;
};

#endif // PART_EFFECTOR_H