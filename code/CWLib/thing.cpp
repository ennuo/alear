#include "thing.h"
#include "ResourceSystem.h"
#include "MatrixUtils.h"
#include <hook.h>

#include <PartSwitch.h>

CThingPtr::~CThingPtr()
{
    Unset();
}

void CThingPtr::Unset()
{
    if (Thing == NULL) return;

    if (Next != NULL) Next->Prev = Prev;
    if (Prev != NULL) Prev->Next = Next;
    else Thing->FirstPtr = Next;

    Thing = NULL;
    Next = NULL;
    Prev = NULL;
}

void CThingPtr::Set(CThing* thing)
{
    Thing = thing;
    if (Thing != NULL)
    {
        Next = thing->FirstPtr;
        thing->FirstPtr = this;
        if (Next != NULL) Next->Prev = this;
        Prev = NULL;
    }
    else
    {
        Next = NULL;
        Prev = NULL;
    }
}

CThingPtr::CThingPtr() : Thing(NULL), Next(NULL), Prev(NULL)
{

}

CThingPtr::CThingPtr(const CThingPtr& rhs) : Thing(NULL), Next(NULL), Prev(NULL)
{
    Set(rhs.Thing);
}

CThingPtr::CThingPtr(const CThingPtr* rhs) : Thing(NULL), Next(NULL), Prev(NULL)
{
    if (rhs)
        Set(rhs->Thing);
}

CThingPtr::CThingPtr(CThing* thing) : Thing(NULL), Next(NULL), Prev(NULL)
{
    Set(thing);
}

CThingPtr& CThingPtr::operator=(CThingPtr const& rhs) 
{ 
    Unset();
    Set(rhs.Thing);
    return *this;
}

CThingPtr& CThingPtr::operator=(CThing* rhs) 
{ 
    Unset();
    Set(rhs);
    return *this;
}

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
    if (type == PART_TYPE_MATERIAL_OVERRIDE)
    {
        PMaterialOverride*& part = CustomThingData->PartMaterialOverride; 
        if (part != NULL) return; 

        part = new PMaterialOverride(); 
        part->SetThing_BECAUSE_I_HATE_CODING_CONVENTIONS_AND_NEED_TO_BE_SPANKED(this); 

        return; 
    }


    CThing_AddPart(this, type);
}

MH_DefineFunc(CThing_RemovePart, 0x000216a4, TOC0, void, CThing*, EPartType);
void CThing::RemovePart(EPartType type)
{
    if (type == PART_TYPE_MATERIAL_OVERRIDE && CustomThingData->PartMaterialOverride != NULL) 
    { 
        delete CustomThingData->PartMaterialOverride; 
        CustomThingData->PartMaterialOverride = NULL; 
    }

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
    Flags = 0;
    CustomThingData = new CCustomThingData();
    ObjectType = OBJECT_UNKNOWN;
}

void CThing::DestroyExtraData()
{
    if (CustomThingData != NULL)
    {
        for (int i = 0; i < CustomThingData->InputList.size(); ++i)
        {
            CSwitchOutput* input = CustomThingData->InputList[i];
            if (input == NULL) continue;
            input->RemoveTarget(this, i);
        }

        if (CustomThingData->PartMaterialOverride != NULL)
            delete CustomThingData->PartMaterialOverride;

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

    if (thing->GetPart(PART_TYPE_SWITCH_KEY) != NULL) return OBJECT_MAGNETIC_KEY;
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
            case SWITCH_TYPE_KEY: return OBJECT_SWITCH_MAGNETIC_LOCK;
            case SWITCH_TYPE_STICKER: return OBJECT_SWITCH_STICKER;
            case SWITCH_TYPE_GRAB: return OBJECT_SWITCH_GRAB;
            case SWITCH_TYPE_PRESSURE: return OBJECT_SWITCH_PRESSURE;
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
            case SWITCH_TYPE_NOP: return OBJECT_SWITCH_NOP;
            case SWITCH_TYPE_MOISTURE: return OBJECT_SWITCH_MOISTURE;
            case SWITCH_TYPE_SIGN_SPLIT: return OBJECT_SWITCH_SIGN_SPLIT;
            case SWITCH_TYPE_ALWAYS_ON: return OBJECT_SWITCH_ALWAYS_ON;
            case SWITCH_TYPE_ANIMATIC: return OBJECT_SWITCH_ANIMATIC;
            case SWITCH_TYPE_SCORE: return OBJECT_SWITCH_SCORE;
            case SWITCH_TYPE_DEATH: return OBJECT_SWITCH_DEATH;
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
            case JOINT_TYPE_MOTOR: 
                if(part_joint->AnimationPattern == 1)
                    return OBJECT_JOINT_MOTOR_BOLT;
                else if(part_joint->AnimationPattern == 2)
                    return OBJECT_JOINT_FLIPPER_BOLT;
                else
                    return OBJECT_JOINT_WOBBLE_BOLT;
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

    if (part_shape != NULL && part_shape->MMaterial)
    {
        switch (part_shape->MMaterial->GetGUID().guid)
        {
            case 0x29e3: return OBJECT_CREATIVE_ZONE;
            case 0x5542: 
            case 0x55fb: 
                return OBJECT_DISSOLVABLE;
            case 0x55fc: return OBJECT_EXPLODING;
            case 0xb123: return OBJECT_MAGICWALL;
            break;
        }
    }

    if (thing->HasPart(PART_TYPE_TRIGGER) && thing->Parent != NULL)
    {
        EObjectType parent_type = GetObjectType(thing->Parent);
        if (parent_type == OBJECT_CREATURE_BRAIN_UNPROTECTED_BUBBLE)
            return OBJECT_CREATURE_BRAIN_UNPROTECTED_BUBBLE_TRIGGER;
        if (parent_type == OBJECT_KEY)
            return OBJECT_KEY_TRIGGER;
        return OBJECT_TRIGGER;
    }

    if (thing->HasPart(PART_TYPE_SCRIPT))
    {
        const CP<RScript>& script = thing->GetPScript()->ScriptInstance.Script;
        if (script)
        {
            switch (script->GetGUID().guid)
            {
                case 0x2d08: return OBJECT_TRIGGER_DELETE;
                case 0x2d0f: return OBJECT_CREDITS_END_LEVEL;
                case 0x3df6: return OBJECT_CREATURE_SOCKET;
                case 0xcb13: return OBJECT_CREDITS_LIGHTS_ON;
                case 0xcb88: return OBJECT_CREDITS_LIGHTS_OFF;
                case 0xcb89: return OBJECT_CREDITS_TEXT;
                case 0x4750: return OBJECT_MUSIC_NORMAL;
                case 0x4563: return OBJECT_POWERUP_JETPACK_TETHERED;
                case 0x2d27: return OBJECT_RACE_END;
                case 0x2d26: return OBJECT_RACE_START;
                case 0xa9c7: return OBJECT_CREATURE_NAVIGATOR;
                case 0x99e2: return OBJECT_CLOSE_LEVEL;
                case 0x6b3c: return OBJECT_TUTORIAL_MONITOR;
                case 0x63ec: return OBJECT_THRUSTER;
                case 0x47f4: return OBJECT_MAGIC_MOUTH;
                case 0x11596: return OBJECT_POWERUP_GRAPPLING_HOOK;
                case 0x10542: return OBJECT_POWERUP_REMOVER;
                case 0x1060d: return OBJECT_MISSILE_EXPLOSIVE;
                case 0x107c1: return OBJECT_BULLET_PLASMA;
                case 0x13534: return OBJECT_SCORE_GIVER;
                case 0x12d39: return OBJECT_BOUNCE_PAD;
                case 0x122d8: return OBJECT_POWERUP_SCUBA_GEAR;
                case 0x11dd9: return OBJECT_BUBBLE_MACHINE;
                case 0x17508: return OBJECT_POWERUP_CREATINATOR_GUN;
                case 0x174fc: return OBJECT_NOTE;
                case 0x15bd7: return OBJECT_IN_OUT_MOVER;
                case 0x154f1: return OBJECT_GAME_ENDER;
                case 0x152bf: return OBJECT_POWERUP_POWER_GLOVES;
                case 0x1e9b7: 
                case 0x1e8f1:
                case 0x26ba4:
                case 0x1e9d0:
                case 0x1e9cf:
                case 0x14616d:
                    return OBJECT_CUSTOM_NOTE;
                case 0x180c1: return OBJECT_LEVEL_LINK;
                case 0x17c2d: return OBJECT_SMOKE_MACHINE;
                case 0x2df80: return OBJECT_PIN_GIVER;
                case 0x2a875: return OBJECT_POWERUP_MOVE;
                case 0x1ab330: return OBJECT_POWERUP_CAPE_COSTUME;
                case 0x1aa497: return OBJECT_POWERUP_CAPE;
                case 0x1148d6: return OBJECT_SPINNING_OBJECT;

                case 16234:
                case 27432:
                    return OBJECT_PRIZE_BUBBLE;
                case 11538: return OBJECT_SCORE_BUBBLE;
                case 17022: return OBJECT_KEY;

                break;
            }
        }
    }

    PGeneratedMesh* part_generated_mesh = thing->GetPGeneratedMesh();
    if (part_generated_mesh != NULL)
    {
        if (part_generated_mesh->GfxMaterial)
        {
            switch (part_generated_mesh->GfxMaterial->GetGUID().guid)
            {
                case 0x18401:
                    return OBJECT_MESH_GENERATED_HOLOGRAM;
                case 0x2b346:
                case 0x307ee:
                    return OBJECT_MESH_GENERATED_STICKER_PANEL;
                case 0x55096:
                    return OBJECT_MESH_GENERATED_DCCOMICS_SPEED;
                case 0x5d405:
                    return OBJECT_MESH_GENERATED_DCCOMICS_LANTERNPOWER;
                case 0x5b94a:
                    return OBJECT_MESH_GENERATED_DCCOMICS_FIREFLAKES;
                case 0x6a257:
                    return OBJECT_MESH_GENERATED_DCCOMICS_INVISIBLE;
                case 0x5789c:
                    return OBJECT_MESH_GENERATED_DCCOMICS_SPACE;
                case 0x464d6:
                    return OBJECT_MESH_GENERATED_TOUCH;
                case 0x139cf:
                    return OBJECT_MESH_GENERATED_SILHOUETTE;
                case 0x139cd:
                    return OBJECT_MESH_GENERATED_OUTLINE;
                case 0x139ce:
                    return OBJECT_MESH_GENERATED_PIXELATE;
                default:
                {
                    // todo: check if color tweakable
                    return OBJECT_MESH_GENERATED;
                }
            }
        }

        return OBJECT_MESH_GENERATED;
    }

    if (part_render_mesh)
        return OBJECT_MESH;

    if (thing->HasPart(PART_TYPE_POS))
        return OBJECT_BONE;

    return OBJECT_UNKNOWN;
}

void CThing::UpdateObjectType()
{
    ObjectType = GetObjectType(this);
}

void CThing::UpdateKeyColours()
{
    if(this == NULL) return;
    PSwitchKey* switch_key = this->GetPSwitchKey();
    if(switch_key != NULL)
    {
        //grab switch colour from script?
        //v4 color = GetSwitchColour(this.switch_key->ColorIndex);
        PRenderMesh* mesh = this->GetPRenderMesh();
        PShape* shape = this->GetPShape();
        //if(shape != NULL)
            //shape->EditorColour = color;
        //else if(mesh != NULL)
            //mesh->EditorColour = color;
    }
}

void CThing::UpdateOldScripts()
{
    if(this == NULL) return;
    if(this->HasPart(PART_TYPE_SCRIPT))
    {
        const CP<RScript>& script = this->GetPScript()->ScriptInstance.Script;
        if (script)
        {
            switch (script->GetGUID().guid)
            {
                case 0x3def:
                    //assign script with new guid
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0x4750));
                    script->Fixup();
                    break;
                case 0x4807:
                case 0x493a:
                case 0x49bb:
                case 0x4bae:
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0x47f4));
                    script->Fixup();
                    break;
                case 0x54fd:
                    this->GetPSwitch()->Type = SWITCH_TYPE_LEVER;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                    break;
                case 0x54fe:
                    this->GetPSwitch()->Type = SWITCH_TYPE_PRESSURE;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                    break;
                case 0x5579:
                    this->GetPSwitch()->Type = SWITCH_TYPE_PROXIMITY;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                    break;
                case 0x5587:
                    this->GetPSwitch()->Type = SWITCH_TYPE_KEY;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                    break;
                case 0x5590:
                    this->GetPSwitch()->Type = SWITCH_TYPE_TRINARY;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                    break;
                case 0x76c0:
                    this->GetPSwitch()->Type = SWITCH_TYPE_STICKER;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                    break;
                case 0x9037:
                    this->GetPSwitch()->Type = SWITCH_TYPE_GRAB;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                    break;
                case 0x9b16:
                    this->GetPSwitch()->Type = SWITCH_TYPE_BUTTON;
                    this->GetPScript()->SetScript(LoadResourceByKey<RScript>(0xa60f));
                    script->Fixup();
                default: return;
            }
        }
    }
    //Remove weird shapes with switches
    else if(this->HasPart(PART_TYPE_SWITCH))
    {
        this->RemovePart(PART_TYPE_SWITCH);
    }
}

void CThing::UpdateOldJoints()
{
    if(this == NULL) return;
    PJoint* joint = this->GetPJoint();
    if(joint != NULL)
    {
        if(joint->Type == JOINT_TYPE_PISTON)
            joint->Stiff = true;
    }
}

void CThing::UpdateOldMeshes()
{
    if(this == NULL) return;
    PRenderMesh* mesh = this->GetPRenderMesh();
    if(mesh == NULL) return;
    PShape* shape = this->GetPShape();
    PPos* pos = this->GetPPos();
    if(shape != NULL)
    {
        switch (mesh->Mesh->GetGUID().guid)
        {
            // Prize Bubble
            case 0x52bc:
            {
                //mesh->Mesh->GetGUID()->SetMesh(LoadResourceByKey<RScript>(0xa60f));
                shape->SetMaterial(LoadResourceByKey<RMaterial>(0x44fd));
                shape->Thickness = 70.0f;
                
                v4 translation; v3 scale; m44 rotation;
                Decompose(translation, rotation, scale, this->GetPPos()->Game.WorldPosition);

                scale.setZ(0.671647f);

                m44 wpos = rotation * m44::scale(scale);
                wpos.setCol3(translation);
                PPos* pos = this->GetPPos();
                pos->Fork->LocalPosition = wpos;
                pos->Fork->WorldPosition = wpos;
                break;
            }
            default:
                break;
        }
    }
}