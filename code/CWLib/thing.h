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
#include "PartJoint.h"
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
#include "PartEmitter.h"
#include "PartCheckpoint.h"
#include "PartPhysicsJoint.h"

#include "hack_thingptr.h"


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
    inline PBody* GetPBody() const { return static_cast<PBody*>(Parts[PART_TYPE_BODY]); }
    inline PJoint* GetPJoint() const { return static_cast<PJoint*>(Parts[PART_TYPE_JOINT]); }
    inline PRenderMesh* GetPRenderMesh() const { return static_cast<PRenderMesh*>(Parts[PART_TYPE_RENDER_MESH]); }
    inline PPos* GetPPos() const { return static_cast<PPos*>(Parts[PART_TYPE_POS]); }
    inline PShape* GetPShape() const { return static_cast<PShape*>(Parts[PART_TYPE_SHAPE]); }
    inline PGeneratedMesh* GetPGeneratedMesh() const { return static_cast<PGeneratedMesh*>(Parts[PART_TYPE_GENERATED_MESH]); }
    inline PWorld* GetPWorld() const { return static_cast<PWorld*>(Parts[PART_TYPE_WORLD]); }
    inline PYellowHead* GetPYellowHead() const { return static_cast<PYellowHead*>(Parts[PART_TYPE_YELLOW_HEAD]); }
    inline PCreature* GetPCreature() const { return static_cast<PCreature*>(Parts[PART_TYPE_CREATURE]); }
    inline PCostume* GetPCostume() const { return static_cast<PCostume*>(Parts[PART_TYPE_COSTUME]); }
    inline PGroup* GetPGroup() const { return static_cast<PGroup*>(Parts[PART_TYPE_GROUP]); }
    inline PLevelSettings* GetPLevelSettings() const { return static_cast<PLevelSettings*>(Parts[PART_TYPE_LEVEL_SETTINGS]); }
    inline PGameplayData* GetPGameplayData() const { return static_cast<PGameplayData*>(Parts[PART_TYPE_GAMEPLAY_DATA]); }
    inline PScriptName* GetPScriptName() const { return static_cast<PScriptName*>(Parts[PART_TYPE_SCRIPT_NAME]); }
    inline PSwitch* GetPSwitch() const { return static_cast<PSwitch*>(Parts[PART_TYPE_SWITCH]); }
    inline PScript* GetPScript() const { return static_cast<PScript*>(Parts[PART_TYPE_SCRIPT]); }
    inline PRef* GetPRef() const { return static_cast<PRef*>(Parts[PART_TYPE_REF]); }
    inline PStickers* GetPStickers() const { return static_cast<PStickers*>(Parts[PART_TYPE_STICKERS]); }
    inline PDecorations* GetPDecorations() const { return static_cast<PDecorations*>(Parts[PART_TYPE_DECORATIONS]); }
    inline PEffector* GetPEffector() const { return static_cast<PEffector*>(Parts[PART_TYPE_EFFECTOR]); }
    inline PEmitter* GetPEmitter() const { return static_cast<PEmitter*>(Parts[PART_TYPE_EMITTER]); }
    inline PCheckpoint* GetPCheckpoint() const { return static_cast<PCheckpoint*>(Parts[PART_TYPE_CHECKPOINT]); }
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