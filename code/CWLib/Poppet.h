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

enum EStampMode {
    STAMP_DEFAULT,
    STAMP_SECONDARY,
    STAMP_SNAP,
    
    NUM_STAMP_MODES
};

class RLocalProfile;

class CPoppet : public CReflectionVisitable {
friend void CustomRaycastAgainstSwitches(CPoppet* poppet);
public:
    void EyedropperPick(CThing* thing);
    void EyedropperPickMesh(CThing* thing);

    void ClearHiddenList();
    void InitializeExtraData();
    void DestroyExtraData();
public:
    const CP<RLocalProfile>& GetLocalProfile() const;
    void RenderHoverObject(CThing* thing, float outline);
    void RenderUI();
    v2 GetBubbleSize();
    EPoppetMode GetMode() const;
    EPoppetSubMode GetSubMode() const;
    bool IsDocked() const;
    v4 GetDockPos() const;
    CThing* GetThingToIgnore();
    void SendPoppetDangerMessage(ELethalType lethal_type);
    void PushMode(EPoppetMode mode, EPoppetSubMode submode);
    void SendPoppetMessage(EPoppetMessageType message);
    void SetDangerType(CThing* thing);
    bool FloodFill(CThing* thing);
    void Backup();
    void ClearMarquee();
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
    char Pad2[0xda4];
public:
    u32 DangerMode;
    CP<RGfxMaterial> FloodFillGfxMaterial;
    CP<RBevel> FloodFillBevel;
    CP<RMaterial> FloodFillPhysicsMaterial;
    u32 FloodFillSoundEnumOverride;
    float FloodFillBevelSize;
    CResourceDescriptor<RPlan> FloodFillMaterialPlan;
private:
    char Pad3[0x3c];
public:
    CRaycastResults Raycast;
    CRaycastResults RaycastForNetwork;
    CThingPtr PlayerThing; // 0x1900
private:
    char Pad4[0x1f4];
public:
    CVector<CThingPtr> HiddenList;
    EStampMode StampMode;
    v2 CustomPoppetSize;
    v2 CustomPoppetOffset;
    bool HidePoppetGooey;
    bool ShowTether;
};

bool CanScaleMesh(CGUID mesh_guid);

#endif // POPPET_H