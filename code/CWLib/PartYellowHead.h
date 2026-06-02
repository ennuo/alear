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

class PYellowHead : public CPart {
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
private:
    char Pad[0x20];
public:
    EPlayerNumber PlayerNumber;
    CThingPtr Head;
    CRenderYellowHead* RenderYellowHead;
    float OnScreenCounter;
    bool OnScreenStatus;
    CPoppet* Poppet;
private:
    char Pad1[0x24];
public:
    u32 SuicideHoldFrameCount;
private:
    char Pad2[0x400];
public:
    u32 LastTimeSlappedAPlayer;
    u32 AnimSetKey;
    bool MonstrousHeadScale;
};

#endif // PART_YELLOW_HEAD_H