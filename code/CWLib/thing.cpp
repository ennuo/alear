#include "thing.h"
#include <hook.h>

MH_DefineFunc(GetWorldAngle, 0x00464f70, TOC1, float, CThing*);

MH_DefineFunc(CThing_ctor, 0x00020128, TOC0, void, CThing*);
CThing::CThing()
{
    CThing_ctor(this);
}

MH_DefineFunc(CThing_dtor, 0x0002250c, TOC0, void, CThing*);
CThing::~CThing()
{
    CThing_dtor(this);
}

MH_DefineFunc(CThing_AddPart, 0x00020770, TOC0, void, CThing*, EPartType);
void CThing::AddPart(EPartType type)
{
    CThing_AddPart(this, type);
}

MH_DefineFunc(CThing_RemovePart, 0x000216a4, TOC0, void, CThing*, EPartType);
void CThing::RemovePart(EPartType type)
{
    CThing_RemovePart(this, type);
}

MH_DefineFunc(CThing_SetWorld, 0x0001f5ec, TOC0, void, CThing*, PWorld*, u32);
void CThing::SetWorld(PWorld* world, u32 preferred_uid)
{
    CThing_SetWorld(this, world, preferred_uid);
}

void CThing::Deflate()
{
    switch (ObjectType)
    {
        case OBJECT_SWITCH_SELECTOR:
        case OBJECT_SWITCH_AND:
        case OBJECT_SWITCH_OR:
        case OBJECT_SWITCH_XOR:
        case OBJECT_SWITCH_NOT:
        case OBJECT_SWITCH_ALWAYS_ON:
        case OBJECT_SWITCH_SIGN_SPLIT:
        case OBJECT_SWITCH_DIRECTION:
            RemovePart(PART_TYPE_RENDER_MESH);
            break;
    }
}

void CThing::InitializeExtraData()
{
    Behaviour = 0;
    ObjectType = OBJECT_UNKNOWN;
    CustomThingData = new CCustomThingData();
}

#include <cell/DebugLog.h>
void CThing::DestroyExtraData()
{
    // DebugLog("destroying thing with uid %d\n", UID);
    // CThingPtr* ptr = FirstPtr;
    // while (ptr != NULL)
    // {
    //     DebugLog("ptr: %08x, next: %08x\n", ptr, ptr->Next);
    //     ptr = ptr->Next;
    // }

    if (CustomThingData != NULL)
    {
        for (int i = 0; i < CustomThingData->InputList.size(); ++i)
        {
            CSwitchOutput* input = CustomThingData->InputList[i];
            if (input == NULL) continue;
            input->RemoveTarget(this, i);
        }
        
        delete CustomThingData;
    }
}

EObjectType GetObjectType(CThing* thing)
{
    if (thing == NULL) return OBJECT_UNKNOWN;

    if (thing->GetPYellowHead() != NULL) return OBJECT_SACKBOY;

    PShape* part_shape = thing->GetPShape();
    if (part_shape != NULL && part_shape->MMaterial && part_shape->MMaterial->GetGUID() == 0x2cbf)
        return OBJECT_HEAD;

    if (thing->GetPart(PART_TYPE_SWITCH_KEY) != NULL) return OBJECT_TAG;
    if (thing->GetPart(PART_TYPE_EMITTER) != NULL) return OBJECT_EMITTER;
    if (thing->GetPart(PART_TYPE_AUDIO_WORLD) != NULL) return OBJECT_SOUND;
    if (thing->GetPart(PART_TYPE_CREATURE) != NULL) return OBJECT_CREATURE_BRAIN_BASE;
    if (thing->GetPart(PART_TYPE_WORLD) != NULL) return OBJECT_WORLD;

    u32 part_flags = 0;
    for (int i = 0; i < PART_TYPE_SIZE; ++i)
    {
        if (thing->GetPart((EPartType)i) != NULL)
            part_flags |= (1 << i);
    }

    if (part_flags == 1 << PART_TYPE_GROUP) return OBJECT_PURE_GROUP;

    PSwitch* part_switch = thing->GetPSwitch();
    if (part_switch != NULL)
    {
        switch (part_switch->Type)
        {
            case SWITCH_TYPE_BUTTON: return OBJECT_SWITCH_BUTTON;
            case SWITCH_TYPE_LEVER: return OBJECT_SWITCH_LEVER;
            case SWITCH_TYPE_TRINARY: return OBJECT_SWITCH_TRINARY;
            case SWITCH_TYPE_PROXIMITY: return OBJECT_SWITCH_PROXIMITY;
            case SWITCH_TYPE_KEY: return OBJECT_SWITCH_KEY;
            case SWITCH_TYPE_STICKER: return OBJECT_SWITCH_STICKER;
            case SWITCH_TYPE_GRAB: return OBJECT_SWITCH_GRAB;
            case SWITCH_TYPE_PAINT: return OBJECT_SWITCH_PAINT;
            case SWITCH_TYPE_PROJECTILE: return OBJECT_SWITCH_PROJECTILE;
            case SWITCH_TYPE_AND: return OBJECT_SWITCH_AND;
            case SWITCH_TYPE_COUNTDOWN: return OBJECT_SWITCH_COUNTDOWN;
            case SWITCH_TYPE_TIMER: return OBJECT_SWITCH_TIMER;
            case SWITCH_TYPE_TOGGLE: return OBJECT_SWITCH_TOGGLE;
            case SWITCH_TYPE_IMPACT: return OBJECT_SWITCH_IMPACT;
            case SWITCH_TYPE_RANDOM: return OBJECT_SWITCH_RANDOM;
            case SWITCH_TYPE_DIRECTION: return OBJECT_SWITCH_DIRECTION;
            case SWITCH_TYPE_OR: return OBJECT_SWITCH_OR;
            case SWITCH_TYPE_XOR: return OBJECT_SWITCH_XOR;
            case SWITCH_TYPE_NOT: return OBJECT_SWITCH_NOT;
            case SWITCH_TYPE_MOISTURE: return OBJECT_SWITCH_MOISTURE;
            case SWITCH_TYPE_SIGN_SPLIT: return OBJECT_SWITCH_SIGN_SPLIT;
            case SWITCH_TYPE_ALWAYS_ON: return OBJECT_SWITCH_ALWAYS_ON;
            case SWITCH_TYPE_ANIMATIC: return OBJECT_SWITCH_ANIMATIC;
            case SWITCH_TYPE_SCORE: return OBJECT_SWITCH_SCORE;
            case SWITCH_TYPE_SELECTOR: return OBJECT_SWITCH_SELECTOR;
            case SWITCH_TYPE_CIRCUIT_BOARD: return OBJECT_SWITCH_CIRCUIT_BOARD;
            case SWITCH_TYPE_CONTROL_PAD: return OBJECT_SWITCH_CONTROL_PAD;
            case SWITCH_TYPE_VITA_PAD: return OBJECT_SWITCH_VITA_PAD;
            case SWITCH_TYPE_CIRCUIT_NODE: return OBJECT_SWITCH_CIRCUIT_NODE;
        }
    }

    PJoint* part_joint = thing->GetPJoint();
    if (part_joint != NULL)
    {
        switch (part_joint->Type)
        {
            case JOINT_TYPE_LEGACY: return OBJECT_JOINT_LEGACY;
            case JOINT_TYPE_ELASTIC: return OBJECT_JOINT_ELASTIC;
            case JOINT_TYPE_SPRING: return OBJECT_JOINT_SPRING;
            case JOINT_TYPE_CHAIN: return OBJECT_JOINT_CHAIN;
            case JOINT_TYPE_PISTON: return OBJECT_JOINT_PISTON;
            case JOINT_TYPE_STRING: return OBJECT_JOINT_STRING;
            case JOINT_TYPE_ROD: return OBJECT_JOINT_ROD;
            case JOINT_TYPE_BOLT: return OBJECT_JOINT_BOLT;
            case JOINT_TYPE_SPRING_ANGULAR: return OBJECT_JOINT_SPRING_BOLT;
            // account for other types of motor bolts later
            case JOINT_TYPE_MOTOR: return OBJECT_JOINT_MOTOR_BOLT;
        }
    }

    PRenderMesh* part_render_mesh = thing->GetPRenderMesh();
    if (part_render_mesh != NULL && part_render_mesh->Mesh)
    {
        CGUID mesh_guid = part_render_mesh->Mesh->GetGUID();
        if (mesh_guid == 0xea9) return OBJECT_SCORE_BUBBLE;
        if (mesh_guid == 0x52bc) return OBJECT_PRIZE_BUBBLE;
        if (mesh_guid == 0x9a96) return OBJECT_CREATURE_BRAIN_PROTECTED_BUBBLE;
        if (mesh_guid == 0x980d) return OBJECT_CREATURE_BRAIN_UNPROTECTED_BUBBLE;
        if (mesh_guid == 0x86c2) return OBJECT_CREATURE_BRAIN_BASE;
    }

    return OBJECT_UNKNOWN;
}

void CThing::UpdateObjectType()
{
    ObjectType = GetObjectType(this);
}

// not technically supposed to be here but i dont care
void CThingPtr::Unset()
{
    if (Thing != NULL)
    {
        if (Next != NULL) Next->Prev = Prev;
        if (Prev == NULL) Thing->FirstPtr = Next;
        else Prev->Next = Next;
    }

    Thing = NULL;
    Next = NULL;
    Prev = NULL;
}

bool CThingPtr::operator<(CThingPtr const& r) const
{
    return Thing->UID < r->UID;
}

void CThingPtr::Set(CThing* thing)
{
    Thing = thing;
    if (Thing != NULL)
    {
        Next = Thing->FirstPtr;
        Thing->FirstPtr = this;
        if (Next != NULL) Next->Prev = this;
        Prev = NULL;
    }
    else
    {
        Next = NULL;
        Prev = NULL;
    }
}