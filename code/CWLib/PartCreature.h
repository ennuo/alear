#ifndef PART_CREATURE_H
#define PART_CREATURE_H

#include <vector.h>
#include <MMAudio.h>

#include "Part.h"
#include "Input.h"
#include "GameEnums.inl"
#include "hack_thingptr.h"
#include "ResourceCharacterSettings.h"

enum EState 
{
    STATE_NORMAL_            = 0,
    STATE_JETPACK           = 1,
    STATE_GUN               = 2,
    STATE_STUNNED           = 3,
    STATE_DIRECT_CONTROL    = 4,
    STATE_DEAD              = 5,
    STATE_LIMBO             = 6,
    STATE_GRAPPLE           = 7,
    STATE_BOOTS             = 8,
    STATE_FORCE             = 9,
    STATE_MOTION_CONTROLLER = 10,
    STATE_DIVER_SUIT = 11,
    STATE_GAS_MASK = 12,
    STATE_FROZEN = 13
};

struct NearestObjectInfo {
    m44 ObjectTransform;
    v2 Pos;
    v2 Normal;
    CThingPtr Object;
    void* Convex;
    float Distance;
    u32 GroundFlag;
    u32 LethalFlag;
    u32 ConvexIndex;
    u32 BestIndex;
    bool Pointy;
    bool StabResult;
};

class PCreature : public CPart {
public:
    class Forked { // 0x480
    public:
        CRawVector<v2> WalkingCachedForce;
        float WalkSpeed;
        float WalkSpeedDesired;
        float WalkAcceleration;
        v2 SwimSpeed;
        v2 SwimSpeedDesired;
        bool IsSwimming;
        float AmountBodySubmerged;
        float AmountHeadSubmerged;
        int JumpFrame;
        CThingPtr JumpingOff;
        v2 JumpDir;
        float ShoveForce;
        v2 JumpAcceleration;
        bool OutOfWaterJumpBoost;
        float SmoothGroundTouch;
        float SmoothHangyness;
        float SmoothedGroundTargetAngle;
        int StickState;
        int WalkAroundCount;
        CVector<NearestObjectInfo> LimbNearestObject;
        v2 GroundNormal; // 0xc0
        v2 GroundNormalRaw;
        v2 GroundNormalSmooth;
        float GroundDistance;
        float MinCollisionZ;
        float MaxCollisionZ;
        float ObstacleMin;
        float ObstacleMax;
        CThingPtr hurt_by[15];
        v2 hurt_force[15];
        v2 hurt_normal[15];
        float RemoveMe;
        v2 GrabNormal;
        //v2 GrabPreviewPos;
        //CThingPtr GrabPreviewThing;
        bool GrabLock;
        bool WaitingToReleaseGrab;
        u32 GrabReleaseTimer;
        CThingPtr GrabJointB;
        v2 GrabJointBContact;
        m44 OldAnimMatrix; // 0x3f0
        float AnimOffset; // 0x430
        bool ZWalkDir; // 0x434
        float BodyAdjustApplied; // 0x438
        v2 WalkInput; // 0x440
        v2 GunDir; // 0x450
        u32 GunFireFrame; // 0x460
        u32 BulletPosIndex; // 0x464
        u32 BulletCount; // 0x468
        u32 BulletImmuneTimer; // 0x46c
        bool GunDirPrecisionMode; //0x470
    };
public:
    void BeSlapped(CThing const* thing, v2 force);
    void SetScubaGear(bool active);
    void SetState(EState state);
    CInput* GetInput();
    bool IsTouchingIce();

    inline u32 GetStateTimer() const { return StateTimer; }
    inline u8 GetState() const { return State; }

public:
    Forked Game;
    Forked Rend;
    Forked* Fork;
    CP<RCharacterSettings> Config;
    u32 ZMode;
    int PlayerAwareness;
    float AwarenessRadius;
    int MoveDirection;
    float SpeedModifier;
    float JumpModifier;
    float StrengthModifier;
    EState State;
    u32 StateTimer;
    CThingPtr GrabJoint;
    u32 Airtime;
    bool HaveNotTouchedGroundSinceUsingJetpack;
    int TypeOfLethalThingTouched;
    v2 LethalForce;
    u32 CrushFrames;
    u32 StunFrames;
    int PostSlapFrames;
    v2 ForceThatSmashedCreature;
private:
    // char Pad2[0xb9];
public:
    bool MeshDirty;
    bool SpawnMessagePending;
    AUDIO_HANDLE JumpSound;
    AUDIO_HANDLE GrabOnSound;
    AUDIO_HANDLE GrabOffSound;
    CVector<CThingPtr> LegList;
    CVector<CThingPtr> EyeList;
    CVector<CThingPtr> BrainAIList;
    CVector<CThingPtr> BrainLifeList;
    CVector<CThingPtr> LifeSourceList;
    CThingPtr LifeCreature;
    CThingPtr AICreature; // 0x9f4
    EPlayerNumber Killer;
    u32 JumpInterval; // 0xa04
    u32 JumpIntervalPhase; // 0xa08
    bool ReactToLethal;
    bool CachedIsEnemy;
    float SwitchScale;
    CThingPtr ResourceThing;
    CThingPtr BulletEmitter; // 0xa20
private:
    char Pad[0x10];
public:
    u32 MaxBulletCount; // 0xa3c
    float AmmoFillFactor; // 0xa40
    u32 AirTimeLeft; // 0xa44
    bool HasBreathedAir; // 0xa48
    bool HasScubaGear; // 2633, 0xa49
    s32 Freeziness;
    s32 WaterTimer;
    float LastForceSum;
    s32 PadForInit;
};

#endif // PART_CREATURE_H