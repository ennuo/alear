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
#include "PartRenderPosition.h"
#include "PartShape.h"
#include "PartGeneratedMesh.h"
#include "PartGroup.h"
#include "PartLevelSettings.h"

class CThingPtr;
class PJoint;
class PBody : public CPart {};
class PPos;

class CThing : public CReflectionVisitable {
public:
    CThing();
    ~CThing();
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
};

extern float (*GetWorldAngle)(CThing* thing);

#include "hack_thingptr.h"

#endif // THING_H