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
private:
    char Pad[0x2b0];
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