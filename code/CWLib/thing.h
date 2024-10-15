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

class CThingPtr;
class PJoint;
class PBody;

class PPos : public CPart {
public:
    u32 AnimHash;
};

class CThing : public CReflectionVisitable {
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
public:
    inline PRenderMesh* GetPRenderMesh() { return static_cast<PRenderMesh*>(Parts[PART_TYPE_RENDER_MESH]); }
    inline PPos* GetPPos() { return static_cast<PPos*>(Parts[PART_TYPE_POS]); }
    inline PWorld* GetPWorld() { return static_cast<PWorld*>(Parts[PART_TYPE_WORLD]); }
    inline PYellowHead* GetPYellowHead() { return static_cast<PYellowHead*>(Parts[PART_TYPE_YELLOW_HEAD]); }
    inline PCreature* GetPCreature() { return static_cast<PCreature*>(Parts[PART_TYPE_CREATURE]); }
    inline PCostume* GetPCostume() { return static_cast<PCostume*>(Parts[PART_TYPE_COSTUME]); }
};

class CThingPtr {
public:
    CThing* Thing;
    CThingPtr* Next;
    CThingPtr* Prev;
public:
    operator CThing*() const { return Thing; }
};

#endif // THING_H