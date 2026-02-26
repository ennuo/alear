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
#include "PartPhysicsJoint.h"
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

#include <PartMaterialOverride.h>
#include <PartList.h>

#include "hack_thingptr.h"

enum EObjectType
{
    #include <ThingObjectType.inl>
    NUM_OBJECT_TYPES
};

class PBody : public CPart {};
class PPos;


class CCustomThingData {
public:
    inline CCustomThingData() : Microchip(), PartMaterialOverride(), InputList()
    {}
public:
    CThing* Microchip;
    PMaterialOverride* PartMaterialOverride;
    CVector<CSwitchOutput*> InputList;
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
    void OnFixup();

    void Deflate();
    void Inflate();
public:
    void SetWorld(PWorld* world, u32 preferred_uid);
    void AddPart(EPartType type);
    void RemovePart(EPartType type);

    CSwitchOutput* GetInput(int port) const;
    void SetInput(CSwitchOutput* input, int port);
    void RemoveInput(int port);
    
    inline int GetInputSize() const 
    {
        if (CustomThingData == NULL) return 0;
        return CustomThingData->InputList.size();
    }

    void UpdateObjectType();
public:
#define PART_MACRO(name, type) inline name* Get##name() const { return (name*)Parts[type]; }
    #include "PartList.h"
#undef PART_MACRO
    inline PMaterialOverride* GetPMaterialOverride() const
    {
        return CustomThingData == NULL ? NULL : CustomThingData->PartMaterialOverride;
    }

    inline bool HasPart(EPartType part) const
    {
        return GetPart(part) != NULL;
    }

    inline CPart* GetPart(EPartType part) const
    {
        if (part == PART_TYPE_MATERIAL_OVERRIDE) return GetPMaterialOverride();
        return Parts[part];
    }
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
private:
    char Pad[0x7];
public:
    u8 ObjectType;
    u8 Behaviour;
    u16 Flags;
    CCustomThingData* CustomThingData;
};

extern float (*GetWorldAngle)(CThing* thing);

#include "hack_thingptr.h"

#endif // THING_H