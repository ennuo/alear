#ifndef RESOURCE_CHARACTER_SETTINGS_H
#define RESOURCE_CHARACTER_SETTINGS_H

#include "Resource.h"

class RCharacterSettings : public CResource {
public:
    void InitializeExtraData();
public:
    int FramesFromFireDeathTillRespawn;
    int FramesFromIceDeathTillRespawn;
    int FramesFromElectricDeathTillRespawn;
    int FramesFromCrushDeathTillRespawn;
    int FramesFromDrownedDeathTillRespawn;
    int FramesTillBurnedToDeath;
    int FramesSinceLastBurnToHeal;
    int FreezeTouchesToFreeze;
    int FramesTillFreezeTouch;
    int FramesTillSlappedToBits;
    float MaxBounceForceFromBurn;
    float ScaleBounceForceFromBurn;
    float ForceToSmashWhenFrozen;
    float ForceToCrush;
    int FramesForCrush;
    float MinVelForStun;
    float ForceToStun;
    int FramesForStun;
    int FramesTillDrown;
    float AmountSubmergedToNotBreath;
    float AmountSubmergedToLosePowerups;
    v4 FrozenBoxScale;
    v4 FrozenBoxOffset;
    float FrozenBoxThick;
    int FramesStartGasEmit;
    int FramesEndGasEmit;
    float MeshScale;
    v2 MeshOffset;
    float HeadScale;
    v2 HeadOffset;
    float GroundTurnSpeed;
    float AirTurnSpeed;
    float MaxDistanceFromGroundForGroundTurnSpeed;
    bool AngleConstrained;
    float ArmCoreHeight;
    float ArmLength;
    float ArmLengthMax;
    float MinStickMoveForArmMovement;
    float ArmUpdateSpeed;
    float ArmUpdateSpeedWhenGrabbing;
    float MinStickLengthForAutoGrabbing;
    float MaxGrabForce;
    float GrabDampening;
    float LegLength;
    float MaxDistForFeetTouching;
    float MinStickMovementForMovement;
    bool WalkForceApplyToCOM;
    float MaxDistanceFromGroundForGroundSpeed;
    float WalkSpeedMaxDesired;
    float WalkSpeedMaxAcceleration;
    float AirSpeedMaxDesired;
    float AirSpeedMaxAcceleration;
    float WaterSpeedMaxDesired;
    float WaterSpeedMaxAcceleration;
    float SwimSpeedMaxDesired;
    float JetpackSpeedMaxDesired;
    float JetpackSpeedMaxAcceleration;
    float FootAnimationSpeed;
    float ArcSizeMultiplier;
    float ArcSizeWhenFlying;
    float HeadTiltWhenMoving;
    float HeadHeightAboveBodyCentre;
    float HeadPositionRightStickWeight;
    float HeadPositionLeftStickWeight;
    float HeadPositionUpdateSpeed;
    bool HandWalking;
    bool Climbing;
    bool RejectSurfacesInLocalSpace;
    float RejectSurfaceThreshold;
    u32 Limbs;
    u32 Arms;
    v2 LimbAnchorPos[4];
    v2 LimbRestPos[4];
    v2 LimbSeparationWhenGrabbing[4];
    v2 PoppetColor[4];
public:
    int FramesTillFreeze;
    int FramesTillFreezeCold;
    int FramesTillFreezeInWater;
    float ForceToFreeze;
    float ForceToShatterOnFreeze;
    float ForceToShatterWhileFrozen;
    int FramesTillMeltInWater;
    int FramesTillFrozenToDeath;
};


#endif // RESOURCE_CHARACTER_SETTINGS_H