#pragma once

#ifdef SPU
    #include <mmtypes.h>
    using namespace std;
#else
    #include <refcount.h>

    #include <Part.h>
    #include <hack_thingptr.h>
    #include <ResourceJoint.h>
#endif

enum JointBehavior
{
    JOINT_BEHAVIOR_ON_OFF,
    JOINT_BEHAVIOR_FORWARDS_BACKWARDS,
    JOINT_BEHAVIOR_SINGLE_CYCLE,
    JOINT_BEHAVIOR_SPEED_SCALE,
    JOINT_BEHAVIOR_POSITIONAL,
    JOINT_BEHAVIOR_COUNT
};

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

#ifndef SPU
class PJoint : public CPart {
struct Forked {
    v2 AccumulatedImpulse[3];
    bool DontRotateA;
};
public:
    inline CThing* GetA() const { return A; }
    inline CThing* GetB() const { return B; }
public:
    float GetMaxLength() const;
    float GetMinLength() const;
    float GetWorldFrame() const;
    float GetCurrentLength() const;
    float GetCurrentAngle() const;

    v4 GetContactPointA() const;
    v4 GetContactPointB() const;

    void SetModScale(float scale);
    void SetPosition(float analogue, bool directional);

    float GetDesiredLength(float frame, bool ignore_mod) const;
    float GetDesiredLengthVel() const;
    float GetMaxDesiredVel() const;
    float GetWaveFactor(float) const;
public:
    u32 Type;
private:
    CThingPtr A;
    CThingPtr B;
public:
    u8 Direction;
    v2 AContact;
    v2 BContact;
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
    bool SpuHack_IsSwitchTriggered;
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
    float SpuHack_Frame;
    float SpuHack_WaveFactor;
    int SpuHack_Behaviour;
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
};
#else
class PJoint {
public:
    float GetDesiredLengthVel() const;
    float GetDesiredLength(float) const;
    float GetWaveFactor(float) const;
public:
    char Pad0[12];
    u32 Type;
    char Pad1[160];
    bool Stiff;
    bool SpuHack_IsSwitchTriggered;
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
    float SpuHack_Frame;
    float SpuHack_WaveFactor;
    int SpuHack_Behaviour;
    char Pad2[132];
    float ModStartFrame;
    float ModDeltaFrames;
    float ModScale;
};
#endif
