#ifndef PART_CHECKPOINT_H
#define PART_CHECKPOINT_H

#include "Part.h"
#include "hack_thingptr.h"

struct SpawnInfo {
    CThingPtr SpawnThing;
    u32 SpawnTimer;
    u32 SpawnPosIndex;
};

class PCheckpoint : public CPart {
public:
    bool Active;
    u32 ActivationFrame;
    int SpawnsLeft;
    int MaxSpawnsLeft;
    bool InstanceInfiniteSpawns;
    float Scroll;
    float Color;
    float Brightness;
    CVector<SpawnInfo> SpawningList;
    u32 SpawnDelay;
    u32 LifeMultiplier;
};

#endif // PART_CHECKPOINT_H