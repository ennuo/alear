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

#include <PartMicrochip.h>
#include <PartMaterialOverride.h>

#include "hack_thingptr.h"

enum EObjectType
{
    OBJECT_UNKNOWN,
    OBJECT_TAG,
    OBJECT_WORLD,
    OBJECT_MICROCHIP,
    OBJECT_SEQUENCER_MUSIC,
    OBJECT_SEQUENCER_GAME,
    OBJECT_CONTROLINATOR,
    OBJECT_MOVINATOR,
    OBJECT_VITANATOR,
    OBJECT_LIGHT,
    OBJECT_EXPLOSIVE_IMPACT,
    OBJECT_EXPLOSIVE_TRIGGERED,
    OBJECT_DISSOLVABLE,
    OBJECT_MESH_GENERATED,
    OBJECT_MESH_GENERATED_TWEAKABLE,
    OBJECT_MESH_GENERATED_HOLOGRAM,
    OBJECT_MESH_GENERATED_STICKER_PANEL,
    OBJECT_MESH_GENERATED_OUTLINE,
    OBJECT_MESH_GENERATED_SILHOUETTE,
    OBJECT_MESH_GENERATED_PIXELATE,
    OBJECT_MESH,
    OBJECT_MESH_ATTACHED_SHAPES,
    OBJECT_MESH_TWEAKABLE,
    OBJECT_BONE,
    OBJECT_PURE_GROUP,
    OBJECT_JOINT_LEGACY,
    OBJECT_JOINT_BOLT,
    OBJECT_JOINT_SPRING_BOLT,
    OBJECT_JOINT_MOTOR_BOLT,
    OBJECT_JOINT_WOBBLE_BOLT,
    OBJECT_JOINT_STRING,
    OBJECT_JOINT_ELASTIC,
    OBJECT_JOINT_SPRING,
    OBJECT_JOINT_ROD,
    OBJECT_JOINT_CHAIN,
    OBJECT_JOINT_PISTON,
    OBJECT_CAMERA_GAME,
    OBJECT_CAMERA_MOVIE,
    OBJECT_CAMERA_STILL,
    OBJECT_SWITCH_BUTTON,
    OBJECT_SWITCH_LEVER,
    OBJECT_SWITCH_TRINARY,
    OBJECT_SWITCH_PROXIMITY,
    OBJECT_SWITCH_KEY,
    OBJECT_SWITCH_STICKER,
    OBJECT_SWITCH_GRAB,
    OBJECT_SWITCH_PAINT,
    OBJECT_SWITCH_PROJECTILE,
    OBJECT_SWITCH_AND,
    OBJECT_SWITCH_COUNTDOWN,
    OBJECT_SWITCH_TIMER,
    OBJECT_SWITCH_TOGGLE,
    OBJECT_SWITCH_IMPACT,
    OBJECT_SWITCH_RANDOM,
    OBJECT_SWITCH_DIRECTION,
    OBJECT_SWITCH_OR,
    OBJECT_SWITCH_XOR,
    OBJECT_SWITCH_NOT,
    OBJECT_SWITCH_MOISTURE,
    OBJECT_SWITCH_SIGN_SPLIT,
    OBJECT_SWITCH_ALWAYS_ON,
    OBJECT_SWITCH_ANIMATIC,
    OBJECT_SWITCH_SCORE,
    OBJECT_SWITCH_SELECTOR,
    OBJECT_SWITCH_CIRCUIT_BOARD,
    OBJECT_SWITCH_CONTROL_PAD,
    OBJECT_SWITCH_VITA_PAD,
    OBJECT_SWITCH_CIRCUIT_NODE,
    OBJECT_LETHALISER,
    OBJECT_DESTROYER,
    OBJECT_SACKBOT_BEHAVIOUR,
    OBJECT_SOUND,
    OBJECT_MUSIC_NORMAL,
    OBJECT_MUSIC_INTERACTIVE,
    OBJECT_INSTRUMENT,
    OBJECT_EMITTER,
    OBJECT_PHYSICS_TWEAK,
    OBJECT_MATERIAL_TWEAK,
    OBJECT_SMOKE_MACHINE,
    OBJECT_THRUSTER,
    OBJECT_BUBBLE_MACHINE,
    OBJECT_GLOBAL_LIGHT_TWEAK,
    OBJECT_GLOBAL_WATER_TWEAK,
    OBJECT_GLOBAL_AUDIO_TWEAK,
    OBJECT_GLOBAL_GRAVITY_TWEAK,
    OBJECT_SCORE_GIVER,
    OBJECT_GAME_ENDER,
    OBJECT_BOUNCE_PAD,
    OBJECT_BOUNCE_PAD_SURFACE,
    OBJECT_MOVER,
    OBJECT_ADVANCED_MOVER,
    OBJECT_FOLLOWER,
    OBJECT_IN_OUT_MOVER,
    OBJECT_GYROSCOPE,
    OBJECT_LOOK_AT_ROTATOR,
    OBJECT_ROCKET_ROTATOR,
    OBJECT_ROTATOR,
    OBJECT_ADVANCED_ROTATOR,
    OBJECT_JOYSTICK_ROTATOR,
    OBJECT_NOTE,
    OBJECT_CUSTOM_NOTE,
    OBJECT_CREATURE_BRAIN_BASE,
    OBJECT_CREATURE_BRAIN_PROTECTED_BUBBLE,
    OBJECT_CREATURE_BRAIN_UNPROTECTED_BUBBLE,
    OBJECT_CREATURE_BRAIN_UNPROTECTED_BUBBLE_TRIGGER,
    OBJECT_CREATURE_WHEEL,
    OBJECT_CREATURE_LEG,
    OBJECT_CREATURE_NAVIGATOR,
    OBJECT_MAGIC_MOUTH,
    OBJECT_MAGIC_EYE,
    OBJECT_ENTRANCE,
    OBJECT_CHECKPOINT_SINGLE_LIFE,
    OBJECT_CHECKPOINT_DOUBLE_LIFE,
    OBJECT_CHECKPOINT_INFINITE_LIFE,
    OBJECT_SCOREBOARD,
    OBJECT_LEVEL_LINK,
    OBJECT_CLOSE_LEVEL,
    OBJECT_SCORE_BUBBLE,
    OBJECT_PRIZE_BUBBLE,
    OBJECT_KEY,
    OBJECT_KEY_TRIGGER,
    OBJECT_RACE_START,
    OBJECT_RACE_START_GATE,
    OBJECT_RACE_END,
    OBJECT_POWERUP_JETPACK_TETHERED,
    OBJECT_POWERUP_JETPACK_TETHERLESS,
    OBJECT_POWERUP_GRAPPLING_HOOK,
    OBJECT_POWERUP_CAPE,
    OBJECT_POWERUP_POWER_GLOVES,
    OBJECT_POWERUP_SCUBA_GEAR,
    OBJECT_POWERUP_PAINTINATOR,
    OBJECT_POWERUP_CREATINATOR,
    OBJECT_POWERUP_CREATINATOR_GUN,
    OBJECT_POWERUP_MOVE,
    OBJECT_POWERUP_MOVE_HANDLE,
    OBJECT_POWERUP_REMOVER,
    OBJECT_SACKBOT,
    OBJECT_SACKBOY,
    OBJECT_HEAD,
    OBJECT_BULLET_WATER,
    OBJECT_BULLET_FIRE,
    OBJECT_BULLET_ELECTRICITY,
    OBJECT_BULLET_PLASMA,
    OBJECT_BULLET_PAINT,
    OBJECT_TUTORIAL_MONITOR,
    OBJECT_BORDER,
    OBJECT_BACKGROUND_ROOT,
    OBJECT_SWITCH_ANGLE,
    OBJECT_SWITCH_VELOCITY_LINEAR,
    OBJECT_SWITCH_VELOCITY_ANGULAR,
    OBJECT_CURSOR,
    OBJECT_SWITCH_MOVINATOR_PAD,
    OBJECT_MOTION_RECORDER,
    OBJECT_PIN_GIVER,
    OBJECT_ATTRACTOR,
    OBJECT_SWITCH_KEY_REMOTE,
    OBJECT_TAG_RADIO,
    OBJECT_WORM_HOLE,
    OBJECT_MESH_GENERATED_TOUCH,
    OBJECT_STATICCLING_TWEAK,
    OBJECT_SWITCH_DATA_SAMPLER,
    OBJECT_SPINNING_OBJECT,
    OBJECT_MESH_GENERATED_DCCOMICS_SPEED,
    OBJECT_MESH_GENERATED_DCCOMICS_LANTERNPOWER,
    OBJECT_MESH_GENERATED_DCCOMICS_FIREFLAKES,
    OBJECT_MESH_GENERATED_DCCOMICS_INVISIBLE,
    OBJECT_MESH_GENERATED_DCCOMICS_SPACE,
    OBJECT_POWERUP_CAPE_COSTUME,
    OBJECT_SWITCH_SCRIPTED,
    OBJECT_TARGET_SCRIPTED,
    NUM_OBJECT_TYPES
};


class PBody : public CPart {};
class PPos;

class CCustomThingData {
public:
    inline CCustomThingData() : Microchip(), PartMicrochip(), PartMaterialOverride(), InputList()
    {}
public:
    CThing* Microchip;
    PMicrochip* PartMicrochip;
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
    inline PMicrochip* GetPMicrochip() const
    {
        return CustomThingData == NULL ? NULL : CustomThingData->PartMicrochip;
    }
    inline PMaterialOverride* GetPMaterialOverride() const
    {
        return CustomThingData == NULL ? NULL : CustomThingData->PartMaterialOverride;
    }

    inline CPart* GetPart(EPartType part) const 
    {
        if (part == PART_TYPE_MICROCHIP) return GetPMicrochip();
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

    // flags & 2 == stamping
};

extern float (*GetWorldAngle)(CThing* thing);

#include "hack_thingptr.h"

#endif // THING_H