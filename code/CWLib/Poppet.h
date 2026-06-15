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

#include <PlayerColours.h>
#include <vector.h>

class CGooeyNodeManager;

struct SDrawObjectEdgesScratch
{
    CRawVector<v2, CAllocatorMMAligned128> Poly;
    CRawVector<unsigned int> SplitAfter;
};

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

class CDotToDotState : public CPoppetChild {
public:
    CDotToDotState();
    ~CDotToDotState();
public:
    void Enter();
    void Render();
    void Update();
    void AddVertex();
public:
    CRawVector<v2, CAllocatorMMAligned128> Polygon;
    CThingPtr PlacementThing;
    float Z;
    float Depth;
};

class CLooksMenuState : public CPoppetChild {
public:
    CLooksMenuState();
    ~CLooksMenuState();
public:
    void Render();
    void Update();
public:
    int Mode;
    int EditBone;
    int EditMorph;
    bool UniformScaling;
};

class RLocalProfile;

class CPoppet : public CReflectionVisitable {
friend void CustomRaycastAgainstSwitches(CPoppet* poppet);
public:
    void EyedropperPick(CThing* thing);
    void EyedropperPickMesh(CThing* thing);
public:
    void ClearHiddenList();
    void InitializeExtraData();
    void DestroyExtraData();

    void RaycastAgainstSwitchConnector(v4 ray_start, v4 ray_dir, CRaycastResults& results);
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
    void PopMode();
    void SendPoppetMessage(EPoppetMessageType message);
    void SetDangerType(CThing* thing);
    bool FloodFill(CThing* thing);
    void Backup();
    void ClearMarquee();
    void DrawObjectEdges(SDrawObjectEdgesScratch& scratch, const CThing* thing, EPlayerColour colour, bool draw_back_and_sides, float fatness, float line_extend, float dotted, float alpha, float scroll_speed) const;
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
    // DONT MOVE THESE!!!
    bool HidePoppetGooey;
    bool ShowTether;
    //

    CLooksMenuState Looks;
    CDotToDotState DotToDot;
    CVector<CThingPtr> HiddenList;
    EStampMode StampMode;
    v2 CustomPoppetSize;
    v2 CustomPoppetOffset;
    NCapture::ESubType CaptureSubType;
};

bool CanScaleMesh(CGUID mesh_guid);

#endif // POPPET_H