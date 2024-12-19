#ifndef POPPET_H
#define POPPET_H

#include <refcount.h>
#include <raycast.h>

#include "thing.h"

#include "ReflectionVisitable.h"
#include "PoppetEnums.inl"
#include "RaycastResults.h"

enum EPoppetStage {
    E_STAGE_LOCKSTEP = 0x4,
    E_STAGE_PREDICTED = 0x8,
    E_STAGE_OTHER = 0x20
};

class RLocalProfile;

// 0x14b4
class CPoppetEditState {
private:
    char Pad[0x3f8];
public:
    CThingPtr LastHoverThing;
};

class CPoppetInventory {
private:
    char Pad[0x80];
public:
    v2 SelectBoxBounds;
};

class CPoppet : public CReflectionVisitable {
friend void CustomRaycastAgainstSwitches(CPoppet* poppet);
public:
    const CP<RLocalProfile>& GetLocalProfile() const;
    void RenderHoverObject(CThing* thing, float outline);
    v2 GetBubbleSize();
    EPoppetMode GetMode() const;
    EPoppetSubMode GetSubMode() const;
    bool IsDocked() const;
    v4 GetDockPos() const;
    CThing* GetThingToIgnore();
    void SendPoppetDangerMessage(ELethalType lethal_type);
    void PushMode(EPoppetMode mode, EPoppetSubMode submode);
    void SendPoppetMessage(EPoppetMessageType message);
private:
    RaycastJobResult m_raycastJobResult;
    CRaycastResults m_raycastOnSwitchConnector;
    CRaycastResults m_raycastResultFromPSwitches;
    v4 m_rayStart;
    v4 m_rayDir;
    float m_bestT;
    float m_bestTFromPSwitches;
public:
    EPoppetStage CurrentStage;
private:
    bool m_havePSwitchHit;
private:
    char Pad[0x463];
public:
    CPoppetEditState Edit; // 0x580
private:
    char Pad1[0x2c];
public:
    CPoppetInventory Inventory; // 0x9b0
private:
    char Pad2[0xec0];
public:
    CThingPtr PlayerThing; // 0x1900
};


#endif // POPPET_H