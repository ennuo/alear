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
    float GetMaxLength() const;
    float GetMinLength() const;
    float GetWorldFrame() const;
    float GetCurrentLength() const;

    v4 GetContactPointA() const;
    v4 GetContactPointB() const;

    void SetModScale(float scale);
    void SetPosition(float analogue, bool directional);
public:
    u32 Type;
    CThingPtr A;
    CThingPtr B;
    u8 Direction;
    v4 AContact;
    v4 BContact;
    float AAngleOffset;
    float BAngleOffset;
    CP<RJoint> Settings;
    u32 LastRenderFrameNum;
    v4 AContactGlobalOld;
    v4 BContactGlobalOld;
    v4 AContactGlobalNew;
    v4 BContactGlobalNew;
    v2 SlideDir;
    bool Stiff;
    float Strength;
    u32 AnimationPattern;
    float AnimationRange;
    float AnimationTime;
    float AnimationPhase;
    float AnimationSpeed;
    float AnimationPause;
    floatInV2 Angle;
    floatInV2 Length;
    u32 JointSoundEnum;
    Forked Game;
    Forked Rend;
    Forked* Fork;
    float ModStartFrame;
    float ModDeltaFrames;
    float ModScale;
    bool ModDriven;
    bool ModScaleActive;
    u32 BoneIdx[2];
    u32 ShapeIdx[2];
    v4 BoneLengths;
    float RenderScale;
    u8 InteractEditMode;
    u8 InteractPlayMode;
    float TweakTargetMaxLength;
    float TweakTargetMinLength;
    bool CurrentlyEditing;
    bool Dissolving;
    floatInV2 OffsetTime;
    bool InvertAngle;
    bool HideInPlayMode;
public:
    void SetA(CThing* thing);
    void SetB(CThing* thing);
public:
    inline CThing* GetA() const { return A; }
    inline CThing* GetB() const { return B; }
};


#endif // PART_PHYSICS_JOINT_H