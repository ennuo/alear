#ifndef PART_EMITTER_H
#define PART_EMITTER_H

#include <vector.h>
#include <refcount.h>

#include "Part.h"

#include <hack_thingptr.h>

class RPlan;
class PGroup;


// 0xf4 / 0x100
class PEmitter : public CPart {
struct Forked {
    float LastUpdateFrame;
};
public:
    CThing* AttemptFire(bool predict, CThing*);
    CThing* Update(bool predict, unsigned int frame);
private:
    CThing* AttemptFireInternal(bool predict, CThing*);
    void UpdateParent();
    void CheckTooManyThingsEmitted();
public:
    Forked Game;
    Forked Rend;
    Forked* Fork;
    v2 PosVel;
    float LinearVel;
    float AngVel;
    u32 Frequency;
    u32 Phase;
    u32 Lifetime;
    CP<RPlan> Plan;
    u32 MaxEmitted;
    u32 CurrentEmitted;
    u32 MaxEmittedAtOnce;
    float ModStartFrame;
    float ModDeltaFrames;
    float ModScale;
    bool ModScaleActive;
    float EmitScale;
    bool EmitFlip;
    CThingPtr ParentThing;
    v4 ParentRelativeOffset;
    float ParentRelativeRotation;
    v4 WorldOffset;
    float WorldRotation;
    float WorldZ;
    float ZOffset;
    float EmitFrontZ;
    float EmitBackZ;
    float StartRotation;
    bool HideInPlayMode;
    CRawVector<CThing*> PredictedWorlds;
    u32 PredictedEmissions;
    CThingPtr LimboWorld;
    CRawVector<PGroup*> EmittedThings;
};


#endif // PART_EMITTER_H