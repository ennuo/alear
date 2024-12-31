#ifndef POPPET_H
#define POPPET_H

#include <refcount.h>
#include <raycast.h>

#include "thing.h"

#include "ReflectionVisitable.h"
#include "PoppetEnums.inl"
#include "RaycastResults.h"
#include "PoppetChild.h"
#include "PoppetEditState.h"
#include "PoppetInventory.h"

enum EPoppetStage {
    E_STAGE_LOCKSTEP = 0x4,
    E_STAGE_PREDICTED = 0x8,
    E_STAGE_OTHER = 0x20
};

class RLocalProfile;

class CPoppet : public CReflectionVisitable {
friend void CustomRaycastAgainstSwitches(CPoppet* poppet);
public:
    void ClearHiddenList();
    void InitializeExtraData();
    void DestroyExtraData();
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
    CPoppetInventory Inventory; // 0x9b0
private:
    char Pad2[0xe20];
public:
    CRaycastResults Raycast;
    CRaycastResults RaycastForNetwork;
    CThingPtr PlayerThing; // 0x1900
private:
    char Pad3[0x1f4];
public:
    CVector<CThingPtr> HiddenList;
};


#endif // POPPET_H