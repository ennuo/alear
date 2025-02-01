#ifndef THING_H
#define THING_H

#include "ReflectionVisitable.h"
#include "part.h"
#include "vector.h"

#include "PartYellowHead.h"
#include "PartCreature.h"
#include "PartCostume.h"
#include "PartRenderMesh.h"
#include "PartPhysicsWorld.h"
#include "PartStickers.h"
#include "PartRenderPosition.h"
#include "PartShape.h"
#include "PartGeneratedMesh.h"
#include "PartGroup.h"
#include "PartLevelSettings.h"
#include "PartGameplayData.h"
#include "PartScriptName.h"
#include "PartScript.h"
#include "PartSwitch.h"
#include "PartRef.h"
#include "PartDecoration.h"
#include "PartEffector.h"

#include "hack_thingptr.h"

class PJoint;
class PBody : public CPart {};
class PPos;

class CCustomThingData {
public:
    inline CCustomThingData()
    {
        memset(this, 0, sizeof(CCustomThingData));
    }
public:
    CThing* Microchip;
};

class CThing : public CReflectionVisitable {
public:
    CThing();
    ~CThing();
public:
    void InitializeExtraData();
    void DestroyExtraData();

    bool HasCustomPartData();
    
    void OnStartSave();
    void OnFinishSave();
        
    ReflectReturn OnLoad();
public:
    void SetWorld(PWorld* world, u32 preferred_uid);
    void AddPart(EPartType type);
    void RemovePart(EPartType type);
public:
    inline PBody* GetPBody() { return static_cast<PBody*>(Parts[PART_TYPE_BODY]); }
    inline PRenderMesh* GetPRenderMesh() { return static_cast<PRenderMesh*>(Parts[PART_TYPE_RENDER_MESH]); }
    inline PPos* GetPPos() { return static_cast<PPos*>(Parts[PART_TYPE_POS]); }
    inline PShape* GetPShape() { return static_cast<PShape*>(Parts[PART_TYPE_SHAPE]); }
    inline PGeneratedMesh* GetPGeneratedMesh() { return static_cast<PGeneratedMesh*>(Parts[PART_TYPE_GENERATED_MESH]); }
    inline PWorld* GetPWorld() { return static_cast<PWorld*>(Parts[PART_TYPE_WORLD]); }
    inline PYellowHead* GetPYellowHead() { return static_cast<PYellowHead*>(Parts[PART_TYPE_YELLOW_HEAD]); }
    inline PCreature* GetPCreature() { return static_cast<PCreature*>(Parts[PART_TYPE_CREATURE]); }
    inline PCostume* GetPCostume() { return static_cast<PCostume*>(Parts[PART_TYPE_COSTUME]); }
    inline PGroup* GetPGroup() { return static_cast<PGroup*>(Parts[PART_TYPE_GROUP]); }
    inline PLevelSettings* GetPLevelSettings() { return static_cast<PLevelSettings*>(Parts[PART_TYPE_LEVEL_SETTINGS]); }
    inline PGameplayData* GetPGameplayData() { return static_cast<PGameplayData*>(Parts[PART_TYPE_GAMEPLAY_DATA]); }
    inline PScriptName* GetPScriptName() { return static_cast<PScriptName*>(Parts[PART_TYPE_SCRIPT_NAME]); }
    inline PSwitch* GetPSwitch() { return static_cast<PSwitch*>(Parts[PART_TYPE_SWITCH]); }
    inline PScript* GetPScript() { return static_cast<PScript*>(Parts[PART_TYPE_SCRIPT]); }
    inline PRef* GetPRef() { return static_cast<PRef*>(Parts[PART_TYPE_REF]); }
    inline PStickers* GetPStickers() { return static_cast<PStickers*>(Parts[PART_TYPE_STICKERS]); }
    inline PDecorations* GetPDecorations() { return static_cast<PDecorations*>(Parts[PART_TYPE_DECORATIONS]); }
    inline PEffector* GetPEffector() { return static_cast<PEffector*>(Parts[PART_TYPE_EFFECTOR]); }
public:
    CThingPtr* FirstPtr;
    CPart* Parts[PART_TYPE_SIZE];
    PWorld* World;
    CThing* FirstChild;
    CThing* NextSibling;
    CThing* Parent;
    CThing* Root;
    CThing* GroupHead;
    CThing* OldEmitter;
    PBody* BodyRoot;
    CRawVector<PJoint*> JointList;
    u32 PlanGUID;
    u32 UID;
    u16 CreatedBy;
    u16 ChangedBy;
    bool Stamping;
    CCustomThingData* CustomThingData;
};

extern float (*GetWorldAngle)(CThing* thing);

#include "hack_thingptr.h"

#endif // THING_H