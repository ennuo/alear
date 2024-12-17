#ifndef PART_PHYSICS_WORLD_H
#define PART_PHYSICS_WORLD_H

#include "Part.h"
#include "vector.h"
#include "hack_thingptr.h"

class PPos;
class PRenderMesh;
class PGeneratedMesh;
class PYellowHead;
class PSwitch;

struct BroadcastMicrochip {
    CThingPtr SourceMicrochip;
    CThingPtr ClonedMicrochip;
};

class RLevel;
class PWorld : public CPart {
public:
    CThing* GetThingByUID(u32 uid);
    CThing* FindThingByScriptName(char const* name);
    void UpgradeAllScripts();
    float GetWaterLevelWorldYCoords() const;
public:
    u32 ThingUIDCounter;
    CRawVector<CThing*> Things;
    RLevel* Level;
private:
    char Pad[0x48];
public:
    CRawVector<PRenderMesh*> ListPRenderMesh;
    CRawVector<PPos*> ListPPos;
    CRawVector<void*> ListPTrigger;
    CRawVector<PYellowHead*> ListPYellowHead;
    CRawVector<void*> ListPAudioWorld;
    CRawVector<void*> ListPAnimation;
    CRawVector<PGeneratedMesh*> ListPGeneratedMesh;
private:
    char Pad5[0xb4];
public:
    CRawVector<PSwitch*> ListPSwitch;
private:
    char Pad6[0x1000];
public:
    CVector<BroadcastMicrochip> BroadcastMicrochips;
    u32 GameMode;
    float GameModeScore[4];
};

#endif // PART_PHYSICS_WORLD_H