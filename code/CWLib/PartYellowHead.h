#ifndef PART_YELLOW_HEAD_H
#define PART_YELLOW_HEAD_H

#include <refcount.h>

#include "RenderYellowHead.h"
#include "PlayerColours.h"
#include "Part.h"
#include "Input.h"

#include "hack_thingptr.h"

class RSyncedProfile;
class RLocalProfile;

class CPoppet;
class CPlayer;

class PYellowHead : public CPart {
public:
    void InitializeExtraData();
public:
    v4 GetActivePosition() const;
    c32 GetColour(EPlayerColour colour);
    void CollectItem(CThing* egg_holding_item, bool new_item);
    const CP<RSyncedProfile>& GetSyncedProfile() const;
    const CP<RLocalProfile>& GetLocalProfile() const;
    CInput* GetInput() const;
    void SetJetpack(CThing* attachment, float length, v2 pos);
public:
    inline CRenderYellowHead* GetRenderYellowHead() const { return RenderYellowHead; }
    CPlayer* GetPlayer();
private:
    char Pad[0x20];
public:
    EPlayerNumber PlayerNumber; // 0x2c
    CThingPtr Head; // 0x30
    CRenderYellowHead* RenderYellowHead; // 0x3c
    float OnScreenCounter; // 0x40
    bool OnScreenStatus; // 0x44
    CPoppet* Poppet; // 0x48
private:
    char Pad1[0x24];
public:
    u32 SuicideHoldFrameCount; // 0x70
private:
    char Pad2[0x48]; // sizeof = 0xbc
public:
    u32 LastTimeSlappedAPlayer;
    u32 AnimSetKey;
    v4 AnimBonePos[64];
    v4 AnimBoneRot[64];
    v4 AnimBoneScale[64];
    float AnimMorph[32];
};

#endif // PART_YELLOW_HEAD_H