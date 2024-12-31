#ifndef POPPET_EDIT_STATE_H
#define POPPET_EDIT_STATE_H

#include <vector.h>

#include "hack_thingptr.h"
#include "StickerInfo.h"
#include "PoppetChild.h"
#include "DecorationInfo.h"
#include "Input.h"
#include "MMAudio.h"
#include "ResourceDescriptor.h"
#include "poppet/PoppetUtilsEditState.h"

class CPoppetEditState : public CPoppetChild {
class CObjectState {
public:
    CVector<CThingPtr> Overlaps;
    CThingPtr Thing;
    int BackZ;
    int FrontZ;
    u32 Flags;
};
class CPlacementBodyState {
public:
    CThingPtr Thing;
    CThingPtr OldParent;
    u32 Frozen;
};
enum {
    BUTTON_HOLD_NONE,
    BUTTON_HOLD_DETAIL,
    BUTTON_HOLD_GLUE,
    BUTTON_HOLD_SMUDGE_STAMP,
    BUTTON_HOLD_SMUDGE_MINUS,
    BUTTON_HOLD_DELETE
};
public:
    CStickerInfo Sticker;
    CDecorationInfo Decoration;
    CThingPtr CursorDummy;
    CVector<CObjectState> EditObjectList;
    CVector<CThingPtr> FrozenList;
    int BackZ;
    int FrontZ;
    float LerpFactor;
    v2 LastSmudgePos;
    u32 LastGridMoveFrame; // 0x230
    u32 LastGridRotateFrame; // 0x234
    u32 LastGridScaleFrame; // 0x238
    u32 ButtonHoldMode;
    EButtonConfig ButtonHoldButton;
    int ButtonHoldFrame;
    CThingPtr ButtonHoldThing;
    bool JustPlacedLongJointEnd;
    AUDIO_HANDLE GluingSound;
    CRawVector<v2,CAllocatorMMAligned128> OverridePolygon; // 0x260
    CRawVector<unsigned int> OverrideLoops; // 0x26c
    int OverrideBack; // 0x278
    int OverrideFront; // 0x27c
    float OverrideScale; // 0x280
    float OverrideAngle; // 0x284
    CP<RGfxMaterial> OverrideGfxMaterial; // 0x288
    CP<RBevel> OverrideBevel; // 0x28c
    CP<RMaterial> OverridePhysicsMaterial; // 0x290
    u32 OverrideSoundEnumOverride; // 0x294
    float OverrideBevelSize; // 0x298
    CResourceDescriptor<RPlan> OverrideMaterialPlan; // 0x29c
    CThingPtr PlacementDummy; // 0x2c4
    CVector<CPlacementBodyState> PlacementBodyState; // 0x2d0
    CVector<CThingPtr> PauseList; // 0x2dc
    CVector<CGlueCandidate> GlueCandidates; // 0x2e8
    CSnapCandidate CurrentBestSnap; // 0x300?
    CSnapCandidate LastBestSnap; // 0x350
private:
    char Pad0[0x10];
public:
    float DecorativeThingAngle; // 0x3b0
    float DecorativeThingScale; // 0x3b4
private:
    char Pad1[0x1c];
public:
    CThingPtr SwitchConnectorRef; // 0x3d4
    CThingPtr SwitchConnector; // 0x3e0
    CThingPtr LastSmudgeThing;
    CThingPtr LastHoverThing;
private:
    char Pad3[0x2c];
};

#endif // POPPET_EDIT_STATE_H