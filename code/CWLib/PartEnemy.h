#ifndef PART_ENEMY_H
#define PART_ENEMY_H

#include <vector.h>
#include <refcount.h>

#include "ResourceDescriptor.h"
#include "Part.h"

#include "hack_thingptr.h"

class PEnemy : public CPart {
public:
    class Forked {
    public:
        float AnimSpeed;
    };
public:
    int PartType;
    CThingPtr CreatureThing;
    bool NewWalkConstraintMass;
    float Radius;
    u32 SnapVertex;
    CThingPtr AnimThing;
    v2 CenterOffset;
private:
    CThingPtr EyeBall;
    CThingPtr Pupil;
    CThingPtr UpperLid;
    CThingPtr LowerLid;
    u32 EyeState;
    CThingPtr TargetThing;
    CThingPtr TargetLookThing;
    v2 TargetOffset;
    int TargetSearchTime;
    v2 EyeBallOffset;
    float Awareness;
    float PupilScale;
    float OpenFactor;
    int BlinkTime;
    int BlinkCount;
    bool AddParticle;
    float ThrusterForce;
    float ThrusterStrength;
    v2 ThrusterDir;
    v2 ThrusterOldPos;
    bool ThrusterOn;
};

#endif // PART_ENEMY_H