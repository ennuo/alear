#ifndef PART_YELLOW_HEAD_H
#define PART_YELLOW_HEAD_H

#include <refcount.h>

#include "RenderYellowHead.h"
#include "PlayerColours.h"
#include "Part.h"
#include "Input.h"

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
private:
    char Pad[0x30];
public:
    CRenderYellowHead* RenderYellowHead;
private:
    char Pad0[0x8];
public:
    CPoppet* Poppet;
private:
    char Pad1[0x24];
public:
    u32 SuicideHoldFrameCount;
};

#endif // PART_YELLOW_HEAD_H