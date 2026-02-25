#ifndef PART_PHYSICS_JOINT_H
#define PART_PHYSICS_JOINT_H

#include <refcount.h>

#include "Part.h"
#include "hack_thingptr.h"
#include "ResourceJoint.h"

enum EJointType 
{
    JOINT_TYPE_LEGACY,
    JOINT_TYPE_ELASTIC,
    JOINT_TYPE_SPRING,
    JOINT_TYPE_CHAIN,
    JOINT_TYPE_PISTON,
    JOINT_TYPE_STRING,
    JOINT_TYPE_ROD,
    JOINT_TYPE_BOLT,

    JOINT_TYPE_FIRST_ANGULAR=7,

    JOINT_TYPE_SPRING_ANGULAR,
    JOINT_TYPE_MOTOR,
    JOINT_TYPE_QUANTIZED,

    JOINT_TYPE_MAX
};

enum EJointPattern {
    JOINT_PATTERN_WAVE,
    JOINT_PATTERN_FORWARDS,
    JOINT_PATTERN_FLIPPER
};

class PJoint : public CPart {
struct Forked {
    v2 AccumulatedImpulse[3];
    bool DontRotateA;
};
public:
    u32 Type;
    CThingPtr A;
    CThingPtr B;
    v2 AContact;
    v2 BContact;
    float AAngleOffset;
    float BAngleOffset;
    CP<RJoint> Settings;
    u32 LastRenderFrameNum;
    v2 AContactGlobalOld;
    v2 BContactGlobalOld;
    v2 AContactGlobalNew;
    v2 BContactGlobalNew;
    v2 SlideDir;
    bool Stiff;
    float Strength;
    u32 AnimationPattern;
    float AnimationRange;
    float AnimationTime;
    float AnimationPhase;
    float AnimationSpeed;
    float AnimationPause;

    // there's a bunch more, but i only really care about these fields.
public:
    void SetA(CThing* thing);
    void SetB(CThing* thing);
public:
    inline CThing* GetA() const { return A; }
    inline CThing* GetB() const { return B; }
};


#endif // PART_PHYSICS_JOINT_H