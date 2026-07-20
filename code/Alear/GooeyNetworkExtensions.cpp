#include "AlearHooks.h"
#include "AlearConfig.h"
#include "CheckpointStyles.h"
#include "ExplosiveStyles.h"
#include "MiscMeshStyles.h"

#include <GooeyNetworkAction.h>
#include <PlayerNumber.inl>

#include <ResourceGame.h>
#include <Translate.h>
#include <ResourceLevel.h>
#include <ResourceGFXTexture.h>
#include <ResourceScript.h>
#include <ResourceTranslationTable.h>
#include <ResourceSystem.h>
#include <PartPhysicsWorld.h>
#include <PartEnemy.h>
#include <thing.h>
#include <mmalex.h>
#include <Launcher.h>

#include <cell/DebugLog.h>
#include <vm/VirtualMachine.h>
#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <scriptobjects/ScriptObjectResource.h>
#include <GameUpdateStage.h>
#include <gooey/GooeyScriptInterface.h>
#include <gooey/GooeyNodeManager.h>
#include <gooey/GooeyCarousel.h>

const u32 E_KEY_TWEAK_SETTINGS_SCRIPT = 3311245350ul;
const u32 E_KEY_GLOBAL_SETTINGS_SDF = 3930801866ul;
const u32 E_KEY_PARAM_ANIMATIONS_SDF = 2295497740ul;
const u32 E_KEY_CHECKPOINT_TYPE_SDF = 3926674768ul;
const u32 E_KEY_VISIBILITY_SDF = 4049006123ul;
const u32 E_KEY_BASIC_ICONS_SDF = 106001ul;
const u32 E_KEY_SENSOR_ICONS_SDF = 107257ul;
const u32 E_KEY_EXPLOSIVE_STYLE_SDF = 4049006123ul;
const u32 E_KEY_INTERACTION_MODE_SDF = 3410918115ul;
const u32 E_KEY_BEVEL_TYPE_SDF = 4077611067u;
const u32 E_KEY_PHYSICS_TYPE_SDF = 3177295135u;

enum ETweakWidget {
    TWEAK_WIDGET_MEASURER,
    TWEAK_WIDGET_CAROUSEL,
    TWEAK_WIDGET_COLOR_INDEX,
    TWEAK_WIDGET_SLIDER,
    TWEAK_WIDGET_COLOR_PICKER,
    TWEAK_WIDGET_KEY_NAME,
    TWEAK_WIDGET_BUTTON,
    TWEAK_WIDGET_STARTPOINT
};

class CIconConfig {
public:
    CIconConfig(u32 key, u32 num_rows, u32 num_columns) : Texture(), NumRows(num_rows), NumColumns(num_columns)
    {
        Texture = LoadResourceByKey<RTexture>(key, 0, STREAM_PRIORITY_DEFAULT);
    }

    CIconConfig(CP<RTexture>& texture, u32 num_rows, u32 num_columns) : Texture(texture), NumRows(num_rows), NumColumns(num_columns)
    {
    }

    v4 GetUV(u32 index)
    {
        f32 x = (f32)(index % NumColumns);
        f32 y = (f32)(index / NumColumns);
        return v4(
            x / ((f32) NumColumns),
            y / ((f32) NumRows),
            (x + 1.0f) / ((f32) NumColumns),
            (y + 1.0f) / ((f32) NumRows)
        );
    }
public:
    CP<RTexture> Texture;
    u32 NumRows;
    u32 NumColumns;
};

// Mimicks the class layout in the VM,
// so we can just cast the member variables
class CTweakSetting {
public:
    CTweakSetting& SetDebugToolTip(const wchar_t* text)
    {
        DebugToolTip = gScriptObjectManager->RegisterStringW(text);
        return *this;
    }

    CTweakSetting& SetDebugSuffix(const wchar_t* text)
    {
        DebugSuffix = gScriptObjectManager->RegisterStringW(text);
        return *this;
    }

    CTweakSetting& SetSuffix(const char* lams_key)
    {
        Suffix = MakeLamsKeyID(lams_key);
        return *this;
    }

    CTweakSetting& SetSuffix(int lams_key)
    {
        Suffix = lams_key;
        return *this;
    }

    CTweakSetting& SetIcon(s32 icon)
    {
        Icon = icon;
        return *this;
    }
    
    CTweakSetting& SetWidget(s32 widget)
    {
        Widget = widget;
        return *this;
    }

    CTweakSetting& SetToolTip(const char* lams_key)
    {
        ToolTip = MakeLamsKeyID(lams_key);
        return *this;
    }

    CTweakSetting& SetToolTip(u32 lams_key)
    {
        ToolTip = lams_key;
        return *this;
    }

    CTweakSetting& SetIcon(const CIconConfig& config, u32 index)
    {
        IconTexture = gScriptObjectManager->RegisterResource(config.Texture.GetRef());

        f32 x = (f32)(index % config.NumColumns);
        f32 y = (f32)(index / config.NumColumns);
        IconUV = v4(
            x / ((f32) config.NumColumns),
            y / ((f32) config.NumRows),
            (x + 1.0f) / ((f32) config.NumColumns),
            (y + 1.0f) / ((f32) config.NumRows)
        );

        return *this;
    }

    CTweakSetting& SetupDistance()
    {
        Widget = TWEAK_WIDGET_MEASURER;
        Icon = LESSMORE_BIG;
        Conversion = 1.0 / 21.0;
        StepSize = 0.1f;
        StepSizeDPad = 1.0f;
        MinValue = 0.0f;
        MaxValue = 5000.0f;

        return *this;
    }

    CTweakSetting& Setup(CIconConfig& config, u32 index, u32 tooltip, f32 min_value, f32 max_value, u32 num_fixed_steps)
    {
        Widget = TWEAK_WIDGET_MEASURER;
        SetIcon(config, index);
        ToolTip = tooltip;
        MinValue = min_value;
        MaxValue = max_value;
        StepSize = (max_value - min_value) / (f32)num_fixed_steps;

        return *this;
    }

    CTweakSetting& Setup(CIconConfig& config, u32 index, u32 tooltip, u32 num_fixed_steps)
    {
        Widget = TWEAK_WIDGET_SLIDER;
        SetIcon(config, index);
        Conversion = 100.0f;
        StepSize = 1.0f;
        StepSizeDPad = 10.0f;
        MinValue = 0.0f;
        MaxValue = 100.0f;
        StepSize = 100.0f / (f32)num_fixed_steps;

        return *this;
    }

    CTweakSetting& SetupCounter()
    {
        Widget = TWEAK_WIDGET_MEASURER;
        StepSize = 1.0f;
        StepSizeDPad = 10.0f;

        return *this;
    }
    
    CTweakSetting& SetupTimer()
    {
        Widget = TWEAK_WIDGET_MEASURER;
        Icon = MEASURER_HOURGLASS;
        Conversion = 1.0f / 30.0f;
        StepSize = 0.1f;
        StepSizeDPad = 1.0f;
        MinValue = 0.0f;
        MaxValue = 86400.0f;
        SetDebugSuffix(L"s");

        return *this;
    }

    CTweakSetting& SetupFraction()
    {
        Widget = TWEAK_WIDGET_MEASURER;
        Conversion = 100.0f;
        StepSize = 1.0f;
        StepSizeDPad = 10.0f;
        SetDebugSuffix(L"%");

        return *this;
    }

    CTweakSetting& SetupYesNo()
    {
        Widget = TWEAK_WIDGET_CAROUSEL;
        Icon = 1;
        return *this;
    }

    CTweakSetting& SetupSlider(s32 type)
    {
        Widget = TWEAK_WIDGET_SLIDER;
        Icon = type;
        Conversion = 100.0f;
        StepSize = 1.0f;
        StepSizeDPad = 10.0f;
        MinValue = 0.0f;
        MaxValue = 100.0f;

        return *this;
    }

    CTweakSetting& SetupSlider()
    {
        return SetupSlider(SLIDER_H);
    }

    CTweakSetting& SetupLighting()
    {
        SetupSlider(SLIDER_H);
        SkipText = true;
        return *this;
    }

    CTweakSetting& SetupLighting(s32 num_steps)
    {
        SetupSlider(SLIDER_H);
        Conversion = (f32)((num_steps - 1) * 10);
        StepSize = 0.1f;
        StepSizeDPad = 10.0f;
        MinValue = 0.0f;
        MaxValue = (f32)((num_steps - 1) * 10);
        SkipText = true;

        return *this;
    }

    CTweakSetting& SetupAngleRange()
    {
        Widget = TWEAK_WIDGET_MEASURER;
        Conversion = 180.0f / M_PI;
        StepSize = 1.0f;
        StepSizeDPad = 15.0f;
        MinValue = 0.0f;
        MaxValue = 360.0f;
        RotateIcon = true;
        
        return *this;
    }

    CTweakSetting& SetupLinearVel()
    {
        Widget = TWEAK_WIDGET_MEASURER;
        Icon = MEASURER_LINEAR_VELOCITY;
        Conversion = 0.142857f;
        StepSize = 0.1f;
        StepSizeDPad = 1.0f;
        return *this;
    }

    CTweakSetting& SetupAngularVel()
    {
        Widget = TWEAK_WIDGET_MEASURER;
        Icon = MEASURER_ANGULAR_VELOCITY;
        Conversion = 1718.873291f;
        StepSize = 1.0f;
        StepSizeDPad = 10.0f;
        MinValue = 0.0f;
        MaxValue = 1500.0f;
        
        return *this;
    }

    CTweakSetting& SetMinMax(f32 min, f32 max)
    {
        MinValue = min;
        MaxValue = max;

        return *this;
    }

    CTweakSetting& SetSteps(f32 step_size, f32 step_size_dpad)
    {
        StepSize = step_size;
        StepSizeDPad = step_size_dpad;
        return *this;
    }

    CTweakSetting& SetConversion(f32 conversion)
    {
        Conversion = conversion;
        return *this;
    }

    CTweakSetting& SetOffset(f32 offset)
    {
        Offset = offset;
        return *this;
    }
    
    CTweakSetting& SetInfinityType(int type)
    {
        InfinityType = type;
        return *this;
    }

    f32 FixedToDisplay(s32 from)
    {
        if (Widget == TWEAK_WIDGET_CAROUSEL) return (s32)from;
        return ((from / 256.0f) * StepSize) + MinValue;
    }

    f32 DisplayToGame(f32 from)
    {
        return (from - Offset) / Conversion;
    }

    f32 GameToDisplay(f32 from)
    {
        from = (from * Conversion) + Offset;
        return from < MinValue ? MinValue : (from > MaxValue ? MaxValue : from);
    }

    s32 DisplayToFixed(f32 from)
    {
        if (Widget == TWEAK_WIDGET_CAROUSEL)
            return (s32)floorf(from + 0.5f);
        return (s32)(floorf(((from - MinValue) / StepSize) + 0.5f) * 256.0f);
    }

    f32 FixedToGame(s32 data)
    {
        return DisplayToGame(FixedToDisplay(data));
    }

    s32 GameToFixed(f32 data)
    {
        return DisplayToFixed(GameToDisplay(data));
    }
public:
    s32 Widget;
    f32 MinValue;
    f32 MaxValue;
    f32 StepSize;
    f32 StepSizeDPad;
    f32 Conversion;
    f32 Power;
    f32 Offset;
    s32 ToolTip;
    bool SkipText;
    s32 Icon;
    s32 InfinityType;
    s32 Suffix;
    ScriptObjectUID IconTexture;
    v4 IconUV;
    bool Centre;
    bool IsSDF;
    bool RotateIcon;
    s32 NumColours;
    s32 SliderColour;
    ScriptObjectUID DebugToolTip;
    ScriptObjectUID DebugSuffix;
};

CScriptObjectInstance* gTweakSettings[E_GOOEY_NETWORK_ACTION_COUNT];

class CMessageGooeyAction {
public:
    u64 NodeID;
    u64 Value;
    EPlayerNumber PlayerNumber;
    int _;
    EGooeyNetworkAction Action;
    int ThingUID;
};

CTweakSetting& GetTweakSetting(EGooeyNetworkAction network_action)
{
    return *(CTweakSetting*)gTweakSettings[network_action]->GetScriptInstance().MemberVariables.begin();
}

float GetWindVelocity()
{
    PWorld* world = gGame->GetWorld();
    PEffector* effector = world->GetThing()->GetPEffector();
    if (effector != NULL)
    {
        return effector->WindStrength;
        // v2& pos_vel = effector->PosVel;

        // float velocity = mmalex::sqrtf((pos_vel.getX() * pos_vel.getX()) + (pos_vel.getY() * pos_vel.getY()));
        
        // velocity = MIN(velocity, 10.0f);
        // velocity = MAX(velocity, 0.0f);

        // return velocity;
    }

    return 0.0f;
}

float GetWindAngle()
{
    PWorld* world = gGame->GetWorld();
    PEffector* effector = world->GetThing()->GetPEffector();
    if (effector != NULL)
    {
        return effector->WindDirection;
        // v2& pos_vel = effector->PosVel;
        
        // float x = pos_vel.getX();
        // float y = pos_vel.getY();

        // if (x == 0.0f && y == 0.0f) return 0.0f;

        // return atan2f(y, x);
    }

    return 0.0f;
}

const float deg2rad = 0.01745329251f;
const float rad2deg = 57.2957795131f;


namespace TweakSettingNativeFunctions
{
    CScriptObjectInstance* Get(EGooeyNetworkAction network_action)
    {
        return gTweakSettings[network_action];
    }

    u32 GetFixedTweakData(EGooeyNetworkAction network_action, u32 thing_uid)
    {
        PWorld* world = gGame->GetWorld();
        if (world == NULL) return 0;
        CThing* thing = world->GetThingByUID(thing_uid);
        if (thing == NULL) return 0;
        CTweakSetting& setting = GetTweakSetting(network_action);

        #define GET_PART_MEMBER(action, part, field_name) \
            case action: \
            { \
                if (thing->Get##part() == NULL) return 0; \
                return setting.GameToFixed(thing->Get##part()->field_name); \
            }

        #define GET_PART_MEMBER_FAST(action, part, field_name) \
            case action: \
            { \
                if (thing->Get##part() == NULL) return 0; \
                return thing->Get##part()->field_name; \
            }

        switch (network_action)
        {
            case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_INVERTED: return thing->GetPSwitch()->Inverted;

            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_ANGLE_RANGE, PSwitch, AngleRange);

            case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_VISIBLE:
            {
                PSwitch* part_switch = thing->GetPSwitch();
                if (part_switch->HideInPlayMode == true && part_switch->HideConnectors == true) return 0;
                if (part_switch->HideInPlayMode == false && part_switch->HideConnectors == true) return 1;
                return 2;
            }

            case E_GOOEY_NETWORK_ACTION_SWITCH_BATTERY: return setting.GameToFixed(thing->GetPSwitch()->ManualActivation.Analogue);
            case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT: return setting.GameToFixed(thing->GetPSwitch()->InputCount);
            case E_GOOEY_NETWORK_ACTION_DEBUG_ACTION0: return setting.GameToFixed(thing->GetPSwitch()->Type);
            case E_GOOEY_NETWORK_ACTION_AND_GATE_OUTPUT:
            case E_GOOEY_NETWORK_ACTION_OR_GATE_OUTPUT:
                return thing->GetPSwitch()->OutputType;

            case E_GOOEY_NETWORK_ACTION_CHECKPOINT_MESH_STYLE: return GetCheckpointStyle(thing);
            case E_GOOEY_NETWORK_ACTION_CHECKPOINT_TYPE: return GetCheckpointType(thing);
            case E_GOOEY_NETWORK_ACTION_CHECKPOINT_DELAY:
            {
                u32 delay = 0;
                PCheckpoint* checkpoint = FindPartCheckpoint(thing);
                if (checkpoint != NULL) delay = checkpoint->SpawnDelay;
                return setting.GameToFixed(delay);
            }
            
            case E_GOOEY_NETWORK_ACTION_TUTORIAL_MODE: return (s32)world->IsTutorialLevel;
            case E_GOOEY_NETWORK_ACTION_LIGHTING:
                return setting.GameToFixed(world->LightingFactor);
            case E_GOOEY_NETWORK_ACTION_DARKNESS:
                return setting.GameToFixed(world->DarknessFactor);
            case E_GOOEY_NETWORK_ACTION_FOGGINESS:
                return setting.GameToFixed(world->FogFactor);
            case E_GOOEY_NETWORK_ACTION_FOGCOLOUR:
                return setting.GameToFixed(world->FogTintFactor);
            case E_GOOEY_NETWORK_ACTION_COLCORRECT:
                return setting.GameToFixed(world->ColorCorrectionFactor);
            case E_GOOEY_NETWORK_ACTION_WATERLEVEL: 
                return setting.GameToFixed(world->WaterLevel);
            case E_GOOEY_NETWORK_ACTION_WATER_WAVE_MAGNITUDE:
                return setting.GameToFixed(world->WaterWaveMagnitude);
            case E_GOOEY_NETWORK_ACTION_WATER_TINT:
                return setting.GameToFixed(world->WaterTint);
            case E_GOOEY_NETWORK_ACTION_WATER_MURKINESS:
                return setting.GameToFixed(world->WaterMurkiness);
            case E_GOOEY_NETWORK_ACTION_WATER_BITS:
                return setting.GameToFixed(world->WaterBits);
            case E_GOOEY_NETWORK_ACTION_GRAVITY:
                return setting.GameToFixed(world->Gravity);
            case E_GOOEY_NETWORK_ACTION_WIND_DIRECTION:
                return setting.GameToFixed(GetWindAngle());
            case E_GOOEY_NETWORK_ACTION_WIND_VELOCITY:
                return setting.GameToFixed(GetWindVelocity());
            case E_GOOEY_NETWORK_ACTION_SHAPE_ANIMATION_SPEED:
            {
                PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
                float speed = mesh != NULL ? mesh->TextureAnimationSpeed : 0.0f;
                return setting.GameToFixed(speed);
            }
            case E_GOOEY_NETWORK_ACTION_MESH_ANIMATION_SPEED:
            {
                PRenderMesh* mesh = thing->GetPRenderMesh();
                CP<RAnim> anim = mesh->Anim;
                float speed = anim != NULL ? mesh->AnimSpeed : 0.0f;
                return setting.GameToFixed(speed);
            }
            
            case E_GOOEY_NETWORK_ACTION_AMMO_COUNT:
            {
                PScript* script = thing->GetPScript();
                s32 bullet_count = script->GetValue<s32>("BulletCount", 0);
                return bullet_count;
            }
            
            case E_GOOEY_NETWORK_ACTION_EXPLOSIVE_STYLE: return GetExplosiveStyle(thing);
            case E_GOOEY_NETWORK_ACTION_LEVEL_KEY_STYLE: return GetLevelKeyStyle(thing);
            case E_GOOEY_NETWORK_ACTION_MAGIC_EYE_STYLE: return GetMagicEyeStyle(thing);
            case E_GOOEY_NETWORK_ACTION_MAGIC_EYE_AWARENESS_RADIUS:
            {
                PEnemy* enemy = thing->GetPEnemy();
                float radius = enemy->Radius;
                return setting.GameToFixed(radius);
            }
            case E_GOOEY_NETWORK_ACTION_LEVER_SWITCH_STYLE: return GetLeverSwitchStyle(thing);
            case E_GOOEY_NETWORK_ACTION_BOUNCE_PAD_STYLE: return GetBouncePadStyle(thing);
            case E_GOOEY_NETWORK_ACTION_SPIKE_PLATE_STYLE: return GetSpikePlateStyle(thing);
            case E_GOOEY_NETWORK_ACTION_INTERACTION_MODE:
            {
                PShape* shape = thing->GetPShape();
                if (shape != NULL)
                    return shape->InteractPlayMode;
                break;
            }
            case E_GOOEY_NETWORK_ACTION_BEVEL_TYPE: return GetBevelType(thing);
            case E_GOOEY_NETWORK_ACTION_BEVEL_SIZE:
            {
                PShape* shape = thing->GetPShape();
                if (shape != NULL)
                    return setting.GameToFixed(shape->BevelSize);
                break;
            }
            case E_GOOEY_NETWORK_ACTION_PHYSICS_TYPE: return GetPhysicsType(thing);
            case E_GOOEY_NETWORK_ACTION_PHYSICS_AUDIO:
            {
                PShape* shape = thing->GetPShape();
                if (shape != NULL)
                    return shape->SoundEnumOverride;
                break;
            }
            case E_GOOEY_NETWORK_ACTION_JUMP_MODIFIER:
            {
                PScript* script = thing->GetPScript();
                f32 jump_modifier = script->GetValue<f32>("JumpModifier", 0.0f);
                DebugLog("Jump Mod: (%f)\n", jump_modifier);
                return setting.GameToFixed(jump_modifier);
            }
            case E_GOOEY_NETWORK_ACTION_LAUNCHER_DISTANCE:
            {
                return setting.GameToFixed(Launcher::GetDistance(thing));
            }
            case E_GOOEY_NETWORK_ACTION_LAUNCHER_ANGLE:
            {
                return setting.GameToFixed(Launcher::GetAngle(thing));
            }
            case E_GOOEY_NETWORK_ACTION_LAUNCHER_AUTO_ACTIVATED:
            {
                PShape* shape = thing->GetPShape();
                if(shape != NULL)
                {
                    shape->EditorColour = Launcher::IsAutoActivated(thing) ? v4(1.0) : v4(0.0, 1.0, 1.0, 1.0) ;
                }
                return !Launcher::IsAutoActivated(thing);
            }
            case E_GOOEY_NETWORK_ACTION_SPEED_MODIFIER:
            {
                PScript* script = thing->GetPScript();
                f32 speed_modifier = script->GetValue<f32>("SpeedModifier", 0.0f);
                DebugLog("Speed Mod: (%f)\n", speed_modifier);
                return setting.GameToFixed(speed_modifier);
            }
            case E_GOOEY_NETWORK_ACTION_ENEMY_COLLECTABLE:
            {
                PCreature* creature = thing->GetPCreature();
                if (creature != NULL)
                {
                    CThing* resource_thing = creature->ResourceThing.GetThing();
                    if (resource_thing != NULL)
                    {
                        return GetCreatureBrainStyle(resource_thing);
                    }
                }

                break;
            }

            case E_GOOEY_NETWORK_ACTION_ENEMY_POINTS:
            {
                PScript* script = thing->GetPScript();
                return script->GetValue<int>("PointsFixed", 0);
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_LENGTH:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                {
                    if (joint->GetThing()->Stamping && joint->TweakTargetMinLength != 0.0)
                        return setting.GameToFixed(joint->TweakTargetMinLength);
                    return setting.GameToFixed((float)joint->Length);
                }
                
                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MIN_LENGTH:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                {
                    if (joint->GetThing()->Stamping && joint->TweakTargetMinLength != 0) 
                        return setting.GameToFixed(joint->TweakTargetMinLength);
                    return setting.GameToFixed(joint->GetMinLength());
                }

                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MAX_LENGTH:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                {
                    if (joint->GetThing()->Stamping && joint->TweakTargetMaxLength != 0) return setting.GameToFixed(joint->TweakTargetMaxLength);
                    return setting.GameToFixed(joint->GetMaxLength());
                }

                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_NUMTURNS:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                    return setting.GameToFixed(abs(joint->AnimationRange));
                break;
            }

            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_TIME, PJoint, AnimationTime);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PAUSE, PJoint, AnimationPause);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PHASE, PJoint, AnimationPhase);

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_ANGLE:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                    return setting.GameToFixed((float)joint->Angle);
                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_SPEED:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                    return setting.GameToFixed(abs(joint->AnimationSpeed));
                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STRENGTH:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                    return setting.GameToFixed(mmalex::powf(SATURATE(joint->Strength), 1.0f / 3.0f));
                break;
            }

            GET_PART_MEMBER_FAST(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STIFF, PJoint, Stiff);

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_DIRECTION:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                {
                    if (joint->AnimationSpeed == 0.0f)
                        return joint->Direction;
                    return 0.0f < joint->AnimationSpeed;
                }

                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_FLIPPER_PISTON:
            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_FLIPPER_WOBBLE:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                {
                    if (joint->AnimationPattern == JOINT_PATTERN_FLIPPER)
                        return 0.0f <= joint->AnimationRange ? 2 : 1;
                    return 0;
                }

                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_REVERSED:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                    return joint->AnimationRange < 0.0f;
                break;
            }

            case E_GOOEY_NETWORK_ACTION_JOINT_BEHAVIOUR:
            {
                return thing->Behaviour;
            }

            case E_GOOEY_NETWORK_ACTION_VISIBLE_IN_PLAY_MODE:
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL)
                    return !joint->HideInPlayMode;
                PEmitter* emitter = thing->GetPEmitter();
                if (emitter != NULL)
                    return !emitter->HideInPlayMode;
                PMicroChip* microchip = thing->GetPMicroChip();
                if (microchip != NULL)
                    return !microchip->HideInPlayMode;

                break;
            }


            GET_PART_MEMBER_FAST(E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_CIRCUIT, PMicroChip, IsCircuitBoardVisible());
            GET_PART_MEMBER_FAST(E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_WIRES, PMicroChip, WiresVisible ^ 1);
            GET_PART_MEMBER_FAST(E_GOOEY_NETWORK_ACTION_MICRO_CHIP_HORIZONTAL_CIRCUIT, PMicroChip, KeepVisualVertical);

            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LINEARVEL, PEmitter, LinearVel);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_ANGULARVEL, PEmitter, AngVel);
            // E_GOOEY_NETWORK_ACTION_EMITTER_IGNORE_PARENTS_VELOCITY
            GET_PART_MEMBER_FAST(E_GOOEY_NETWORK_ACTION_EMITTER_BEHAVIOUR, PEmitter, GetThing()->Behaviour);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_FREQUENCY, PEmitter, Frequency);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LIFETIME, PEmitter, Lifetime);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_PHASE, PEmitter, Phase);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXNUM, PEmitter, MaxEmitted);
            GET_PART_MEMBER(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXATONCENUM, PEmitter, MaxEmittedAtOnce);
            // E_GOOEY_NETWORK_ACTION_EMITTER_RECYCLE
            // E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_CREATE
            // E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_DESTROY

        }

        #undef GET_PART_MEMBER

        return 0;
    }

    f32 GetStylesheetScalingFactor(CScriptObjectGooey* gooey)
    {
        if (gooey == NULL) return 0.0f;
        CScriptedGooeyWrapper* wrapper = gooey->GetNativeObject();
        if (wrapper == NULL) return 0.0f;
        return wrapper->Manager->StylesheetScalingFactor;
    }

    bool UseLegacyKeyColors()
    {
        return gUseLegacyKeyColors;
    }

    bool UseNewKeyColorSelection()
    {
        return gUseNewKeyColorSelection;
    }
    

    bool DebugMaterialTweaks()
    {
        return gDebugMaterialTweaks;
    }

    bool SeparateToys()
    {
        return gUseToysPage;
    }

    bool UsePaintPage()
    {
        return gUsePaintPage;
    }

    void Register()
    {
        RegisterNativeFunction("TweakSetting", "GetTweakSetting__i", true, NVirtualMachine::CNativeFunction1<CScriptObjectInstance*, EGooeyNetworkAction>::Call<Get>);
        RegisterNativeFunction("TweakSetting", "GetFixedTweakData__ii", true, NVirtualMachine::CNativeFunction2<u32, EGooeyNetworkAction, u32>::Call<GetFixedTweakData>);
        RegisterNativeFunction("Gooey", "GetStylesheetScalingFactor__", false, NVirtualMachine::CNativeFunction1<f32, CScriptObjectGooey*>::Call<GetStylesheetScalingFactor>);
        RegisterNativeFunction("PlayerColour", "UseLegacyKeyColors__", true, NVirtualMachine::CNativeFunction0<bool>::Call<UseLegacyKeyColors>);
        RegisterNativeFunction("PlayerColour", "UseNewKeyColorSelection__", true, NVirtualMachine::CNativeFunction0<bool>::Call<UseNewKeyColorSelection>);
        RegisterNativeFunction("TweakShape", "AllowDebugTweaks__", true, NVirtualMachine::CNativeFunction0<bool>::Call<DebugMaterialTweaks>);
        RegisterNativeFunction("Poppet", "SeparateToys__", true, NVirtualMachine::CNativeFunction0<bool>::Call<SeparateToys>);
        RegisterNativeFunction("Poppet", "UsePaintPage__", true, NVirtualMachine::CNativeFunction0<bool>::Call<UsePaintPage>);
    }
}

bool AllocateTweakSettings()
{
    CMainGameStageOverride _stage_override(E_UPDATE_STAGE_OTHER_WORLD);

    CThing* thing = new CThing();
    thing->AddPart(PART_TYPE_WORLD);
    PWorld* world = thing->GetPWorld();

    CP<RScript> script = LoadResourceByKey<RScript>(E_KEY_TWEAK_SETTINGS_SCRIPT, 0, STREAM_PRIORITY_DEFAULT);
    script->BlockUntilLoaded();
    if (!script->IsLoaded()) return false;

    for (int i = 0; i < E_GOOEY_NETWORK_ACTION_COUNT; ++i)
    {
        CScriptObjectInstance* instance = CScriptObjectInstance::Create(script, world, true);
        gTweakSettings[i] = instance;
        gScriptObjectManager->AddRoot(instance);
    }

    delete thing;
    return true;
}

bool InitTweakSettings()
{
    if (!AllocateTweakSettings()) return false;

    CIconConfig global_settings_texture(E_KEY_GLOBAL_SETTINGS_SDF, 4, 4);
    CIconConfig paramanim_texture(E_KEY_PARAM_ANIMATIONS_SDF, 1, 2);
    CIconConfig visibleinplaymode_icon_texture(106058u, 1, 1);
    CIconConfig sensor_icons(E_KEY_SENSOR_ICONS_SDF, 4, 4);
    CIconConfig explosive_style_texture(E_KEY_EXPLOSIVE_STYLE_SDF, 2, 4);
    CIconConfig interaction_mode_texture(E_KEY_INTERACTION_MODE_SDF, 2, 2);
    CIconConfig general_icons_texture(30648u, 4, 4);
    CIconConfig physics_type_texture(E_KEY_PHYSICS_TYPE_SDF, 4, 4);
    CIconConfig bevel_type_texture(E_KEY_BEVEL_TYPE_SDF, 4, 4);
    CIconConfig tweak_joint_icons(106057u, 4, 4);
    CIconConfig tweak_input_action_icons(107100u, 2, 4);
    CIconConfig tweak_inputs_icon(108529, 1, 1);
    CIconConfig tweak_visibility_icons(108529, 1, 2);
    CIconConfig tweak_batteryoutput_icon(119887, 1, 1);
    CIconConfig tweak_danger_icons(107868, 2, 4);
    CIconConfig tweak_timing_icons(106137, 2, 2);
    CIconConfig tweak_emitter_icons(39196, 4, 4);

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_LIGHTING)
        .SetupLighting()
        .SetIcon(SLIDER_LIGHTING)
        .SetIcon(global_settings_texture, 0)
        .SetToolTip("LIGHTING");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_DARKNESS)
        .SetupLighting()
        .SetIcon(global_settings_texture, 1)
        .SetToolTip("DARKNESS_FACTOR");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_FOGGINESS)
        .SetupLighting()
        .SetIcon(global_settings_texture, 2)
        .SetToolTip("FOG_FACTOR");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_FOGCOLOUR)
        .SetupLighting(10)
        .SetIcon(global_settings_texture, 3)
        .SetConversion(80.0f)
        .SetToolTip("FOG_TINT_FACTOR");
    
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_COLCORRECT)
        .SetupLighting()
        .SetIcon(global_settings_texture, 4)
        .SetMinMax(0.0f, 700.0f)
        .SetSteps(10.0f, 100.0f)
        .SetToolTip("COLOR_CORRECTION");
        

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_WATERLEVEL)
        .SetupDistance()
        .SetIcon(global_settings_texture, 5)
        .SetConversion(40400.0 / 210.0)
        .SetMinMax(0.0f, 40400 / 210.0)
        .SetToolTip("WATER_LEVEL");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_WATER_WAVE_MAGNITUDE)
        .SetupDistance()
        .SetIcon(global_settings_texture, 6)
        .SetSteps(0.01f, 0.1f)
        .SetConversion(1.2f)
        .SetMinMax(0.0f, 2.0f)
        .SetToolTip("WAVES");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_WATER_TINT)
        .SetupLighting()
        .SetIcon(global_settings_texture, 7)
        .SetToolTip("WATER_COLOUR")
        .SetDebugSuffix(L"%");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_WATER_MURKINESS)
        .SetupSlider()
        .SetIcon(global_settings_texture, 8)
        .SetToolTip("WATER_MURKINESS")
        .SetDebugSuffix(L"%");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_WATER_BITS)
        .SetupSlider()
        .SetIcon(global_settings_texture, 9)
        .SetToolTip("WATER_BITS")
        .SetDebugSuffix(L"%");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_GRAVITY)
        .SetupSlider()
        .SetIcon(global_settings_texture, 10)
        .SetMinMax(0.0f, 400.0f)
        .SetToolTip("GRAVITY")
        .SetDebugSuffix(L"%");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_WIND_VELOCITY)
        .SetupSlider()
        .SetIcon(global_settings_texture, 12)
        .SetToolTip("WIND_VELOCITY")
        .SetConversion(1.0 / 50)
        // .SetConversion(1.0 / 100)
        .SetDebugSuffix(L"%");
    
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_WIND_DIRECTION)
        .SetupAngleRange()
        .SetIcon(global_settings_texture, 13)
        .SetOffset(-90.0f)
        .SetToolTip("WIND_DIRECTION");
    
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_LOGIC_MODE)
        .SetupYesNo()
        .SetDebugToolTip(L"Use Advanced Logic System");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_TUTORIAL_MODE)
        .SetupYesNo()
        .SetIcon(global_settings_texture, 15)
        .SetDebugToolTip(L"Is Tutorial");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SHAPE_ANIMATION_SPEED)
        .SetupFraction()
        .SetIcon(paramanim_texture, 1)
        .SetMinMax(-200.0f, 200.0f)
        .SetDebugToolTip(L"Animation Speed");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_MESH_ANIMATION_SPEED)
        .SetupFraction()
        .SetIcon(paramanim_texture, 1)
        .SetMinMax(-200.0f, 200.0f)
        .SetDebugToolTip(L"Animation Speed");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_CHECKPOINT_MESH_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_MESH_STYLE)
        .SetDebugToolTip(L"Visual Style");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_CHECKPOINT_TYPE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_CHECKPOINT)
        .SetDebugToolTip(L"Checkpoint Type");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_CHECKPOINT_DELAY)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(0.0, 300.0f)
        .SetSteps(1.0f, 10.0f)
        .SetDebugSuffix(L"s")
        .SetDebugToolTip(L"Spawn Delay")
        .SetConversion(1.0 / 30.0);
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_INVERTED)
        .SetupYesNo()
        .SetIcon(sensor_icons, 4)
        .SetDebugToolTip(L"Invert Output");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_VISIBLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(visibleinplaymode_icon_texture, 0)
        .SetIcon(CAROUSEL_SWITCH_VISIBILITY)
        .SetDebugToolTip(L"Electronics and Cable Visibility");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_BATTERY)
        .SetupFraction()
        .SetIcon(tweak_batteryoutput_icon, 0)
        .SetMinMax(-100.0f, 100.0f)
        .SetDebugToolTip(L"Output");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT)
        .SetupCounter()
        .SetIcon(tweak_inputs_icon, 0)
        .SetMinMax(1.0f, 10.0f)
        .SetSteps(1.0f, 1.0f)
        .SetDebugToolTip(L"Number of Ports");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_SELECTOR_OUTPUT)
        .SetupCounter()
        .SetIcon(sensor_icons, 3)
        .SetOffset(1.0f)
        .SetMinMax(1.0f, 10.0f)
        .SetSteps(1.0f, 1.0f)
        .SetDebugToolTip(L"Current State");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_DEBUG_ACTION0)
        .SetupCounter()
        .SetIcon(tweak_inputs_icon, 0)
        .SetMinMax(SWITCH_TYPE_AND, NUM_SWITCH_TYPES - 1)
        .SetSteps(1.0f, 1.0f)
        .SetDebugToolTip(L"DEV ONLY! Switch Type");


    // GetTweakSetting(E_GOOEY_NETWORK_ACTION_AND_GATE_OUTPUT)
    //     .SetWidget(TWEAK_WIDGET_CAROUSEL)
    //     .SetIcon(sensor_icons, 9)
    //     .SetIcon(CAROUSEL_AND_GATE_OUTPUT)
    //     .SetDebugToolTip(L"Output Value");

    // GetTweakSetting(E_GOOEY_NETWORK_ACTION_OR_GATE_OUTPUT)
    //     .SetWidget(TWEAK_WIDGET_CAROUSEL)
    //     .SetIcon(sensor_icons, 9)
    //     .SetIcon(CAROUSEL_OR_GATE_OUTPUT)
    //     .SetDebugToolTip(L"Output Value");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_ANGLE_RANGE)
        .SetupAngleRange()
        .SetIcon(sensor_icons, 1)
        .SetIcon(MEASURER_ANGLE_RANGE)
        .SetConversion(2.0f)
        .SetDebugToolTip(L"Trigger Angle Range");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_AMMO_COUNT)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(0.0, 300.0f)
        .SetSteps(1.0f, 10.0f)
        .SetDebugToolTip(L"Paint Amount");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EXPLOSIVE_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_EXPLOSIVE_STYLE)
        .SetIcon(explosive_style_texture, 0)
        .SetDebugToolTip(L"Explosive Type");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_LEVEL_KEY_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_LEVEL_KEY_STYLE)
        .SetIcon(explosive_style_texture, 0)
        .SetDebugToolTip(L"Visual Style");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_MAGIC_EYE_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_MAGIC_EYE_STYLE)
        .SetToolTip("ENEMY_AWARENESS_RADIUS");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_MAGIC_EYE_AWARENESS_RADIUS)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(0.0, 2000.0f)
        .SetSteps(1.0f, 10.0f)
        .SetIcon(paramanim_texture, 1)
        .SetDebugToolTip(L"Awareness Radius");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_LEVER_SWITCH_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_MESH_STYLE)
        .SetDebugToolTip(L"Visual Style");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_BOUNCE_PAD_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_MESH_STYLE)
        .SetDebugToolTip(L"Visual Style");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SPIKE_PLATE_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_MESH_STYLE)
        .SetDebugToolTip(L"Visual Style");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCHKEY_TWEAK_VISIBLE)
        .SetupYesNo()
        .SetIcon(visibleinplaymode_icon_texture, 0)
        .SetToolTip("HIDE_IN_PLAY_MODE");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_INTERACTION_MODE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_INTERACTION_MODE)
        .SetIcon(interaction_mode_texture, 0)
        .SetDebugToolTip(L"INTERACTION_MODE");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_BEVEL_TYPE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_BEVEL_TYPE)
        .SetIcon(bevel_type_texture, 0)
        .SetDebugToolTip(L"Bevel Type");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_BEVEL_SIZE)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetIcon(bevel_type_texture, 1)
        .SetMinMax(0.0f, 100.0f)
        .SetSteps(0.1f, 1.0f)
        .SetDebugToolTip(L"Bevel Size");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_PHYSICS_TYPE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_PHYSICS_TYPE)
        .SetIcon(physics_type_texture, 0)
        .SetDebugToolTip(L"Physics Material");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_PHYSICS_AUDIO)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_PHYSICS_AUDIO)
        .SetIcon(general_icons_texture, 1)
        .SetDebugToolTip(L"Physics Audio");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JUMP_MODIFIER)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(1.0f, 2.0f)
        .SetSteps(0.01f, 0.1f)
        .SetIcon(paramanim_texture, 1)
        .SetDebugToolTip(L"JUMP_MODIFIER");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_LAUNCHER_DISTANCE)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(1.0f, 100.0f)
        .SetSteps(0.1f, 1.0f)
        .SetIcon(paramanim_texture, 1)
        .SetDebugToolTip(L"Launcher Height");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_LAUNCHER_ANGLE)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(-45.0f, 45.0f)
        .SetSteps(1.0f, 5.0f)
        .SetIcon(paramanim_texture, 1)
        .SetDebugToolTip(L"Launcher Angle");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_LAUNCHER_AUTO_ACTIVATED)
        .SetupYesNo()
        .SetDebugToolTip(L"Manual Activation");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SPEED_MODIFIER)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(1.0f, 2.0f)
        .SetSteps(0.01f, 0.1f)
        .SetIcon(paramanim_texture, 1)
        .SetDebugToolTip(L"SPEED_MODIFIER");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_ENEMY_COLLECTABLE)
        .SetupYesNo()
        .SetIcon(paramanim_texture, 1)
        .SetToolTip("ENEMY_COLLECTABLE");
        
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_ENEMY_POINTS)
        .SetWidget(TWEAK_WIDGET_MEASURER)
        .SetMinMax(0.0, 1000.0f)
        .SetSteps(1.0f, 10.0f)
        .SetIcon(paramanim_texture, 1)
        .SetToolTip("ENEMY_POINTS_AWARDS");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_LENGTH)
        .SetupDistance()
        .SetIcon(tweak_joint_icons, 0)
        .SetIcon(LESSMORE_BIG)
        .SetToolTip(3557118262ul);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MIN_LENGTH)
        .SetupDistance()
        .SetIcon(tweak_joint_icons, 1)
        .SetIcon(LESSMORE_BIG)
        .SetToolTip(1029670008);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MAX_LENGTH)
        .SetupDistance()
        .SetIcon(tweak_joint_icons, 0)
        .SetIcon(LESSMORE_BIG)
        .SetToolTip(88516986);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_NUMTURNS)
        .SetupAngleRange()
        .SetMinMax(0.0f, 360.0f * 6.0f)
        .SetIcon(tweak_joint_icons, 14)
        .SetIcon(LESSMORE_TURNS)
        .SetToolTip(732815004);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_TIME)
        .SetupTimer()
        .SetIcon(tweak_timing_icons, 0)
        .SetToolTip(4039811215ul);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PAUSE)
        .SetupTimer()
        .SetIcon(tweak_timing_icons, 1)
        .SetToolTip(2207054154ul);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PHASE)
        .SetupTimer()
        .SetIcon(tweak_timing_icons, 2)
        .SetToolTip(2207054154ul);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_ANGLE)
        .SetupAngleRange()
        .SetIcon(tweak_joint_icons, 12)
        .SetToolTip(950047607);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_SPEED)
        .SetupAngularVel()
        .SetIcon(tweak_joint_icons, 15)
        .SetToolTip(3975976167ul);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STRENGTH)
        .SetupFraction()
        .SetIcon(tweak_joint_icons, 3)
        .SetToolTip(192121499);
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STIFF)
        .SetupYesNo()
        .SetIcon(tweak_joint_icons, 2)
        .SetToolTip(2048456740);

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_DIRECTION)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(tweak_joint_icons, 13)
        .SetIcon(CAROUSEL_BOLT_DIRECTION)
        .SetDebugToolTip(L"Direction");
    // E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_FLIPPER_PISTON
    // E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_FLIPPER_WOBBLE
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_REVERSED)
        .SetupYesNo()
        .SetIcon(tweak_joint_icons, 5)
        .SetToolTip(156327334);

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_JOINT_BEHAVIOUR)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(tweak_input_action_icons, 0)
        .SetIcon(CAROUSEL_JOINT_BEHAVIOUR)
        .SetDebugToolTip(L"Input Action");


    GetTweakSetting(E_GOOEY_NETWORK_ACTION_VISIBLE_IN_PLAY_MODE)
        .SetupYesNo()
        .SetIcon(visibleinplaymode_icon_texture, 0)
        .SetDebugToolTip(L"Visible in Play Mode");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_CIRCUIT)
        .SetupYesNo()
        .SetIcon(CIconConfig(106055, 1, 1), 0)
        .SetDebugToolTip(L"Show Circuitboard");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_MICRO_CHIP_HORIZONTAL_CIRCUIT)
        .SetupYesNo()
        .SetIcon(CIconConfig(818522, 4, 4), 13)
        .SetDebugToolTip(L"Stabilize Circuitboard");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_WIRES)
        .SetupYesNo()
        .SetIcon(CIconConfig(108528, 1, 1), 0)
        .SetDebugToolTip(L"Hide Outgoing Wires when Circuitboard is Closed");


    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LINEARVEL)
        .SetupLinearVel()
        .SetToolTip(2638927608ul)
        .SetIcon(tweak_emitter_icons, 0);

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_ANGULARVEL)
        .SetupAngularVel()
        .SetMinMax(-1500.0f, 1500.0f)
        .SetToolTip(1328326009)
        .SetIcon(tweak_emitter_icons, 1);
    
    // E_GOOEY_NETWORK_ACTION_EMITTER_IGNORE_PARENTS_VELOCITY
    
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_BEHAVIOUR)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(CAROUSEL_EMITTER_BEHAVIOUR)
        .SetIcon(tweak_input_action_icons, 0)
        .SetDebugToolTip(L"Input Action");


    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_FREQUENCY)
        .SetupTimer()
        .SetIcon(MEASURER_SPEEDO)
        .SetToolTip(3035953324ul)
        .SetIcon(tweak_emitter_icons, 3);
    
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LIFETIME)
        .SetupTimer()
        .SetIcon(MEASURER_LIFE_SPAN)
        .SetToolTip(473723627ul)
        .SetInfinityType(1)
        .SetIcon(tweak_emitter_icons, 4);
    
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_PHASE)
        .SetupTimer()
        .SetIcon(MEASURER_PHASE)
        .SetToolTip(3075593739ul)
        .SetIcon(tweak_timing_icons, 2);

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXNUM)
        .SetupCounter()
        .SetIcon(MEASURER_MAX_EMITTED_EVER)
        .SetToolTip(2846760626)
        .SetInfinityType(1)
        .SetIcon(tweak_emitter_icons, 5);
    
    GetTweakSetting(E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXATONCENUM)
        .SetupCounter()
        .SetIcon(MEASURER_MAX_EMITTED_ATONCE)
        .SetToolTip(3642780210ull)
        .SetIcon(tweak_emitter_icons, 6);

    // E_GOOEY_NETWORK_ACTION_EMITTER_RECYCLE
    // E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_CREATE
    // E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_DESTROY

    
    return true;
}

void DoNetworkActionResponse(CMessageGooeyAction& action)
{
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;

    CTweakSetting& setting = GetTweakSetting(action.Action);

    #define GET_PART(part_type) \
        CThing* thing = world->GetThingByUID(action.ThingUID); \
        if (thing == NULL) break; \
        part_type* part = thing->Get##part_type(); \
        if (part == NULL) break; \
        part

    switch (action.Action)
    {
        case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_ANGLE_RANGE:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();
            if (part_switch != NULL) 
                part_switch->AngleRange = setting.FixedToGame(action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_INVERTED:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();
            if (part_switch != NULL) part_switch->Inverted = action.Value;
            break;
        }

        case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_VISIBLE:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();
            if (part_switch != NULL)
            {
                switch (action.Value)
                {
                    case 0:
                    {
                        part_switch->HideConnectors = true;
                        part_switch->HideInPlayMode = true;
                        
                        break;
                    }
                    case 1:
                    {
                        part_switch->HideConnectors = true;
                        part_switch->HideInPlayMode = false;
                        break;
                    }
                    case 2:
                    {
                        part_switch->HideConnectors = false;
                        part_switch->HideInPlayMode = false;
                    }

                }
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_SWITCH_BATTERY:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();
            if (part_switch != NULL)
            {
                float analogue = setting.FixedToGame(action.Value);
                part_switch->ManualActivation.Analogue = analogue;
                part_switch->ManualActivation.Ternary = analogue < 0.0f ? -1 : analogue > 0.0f ? 1 : 0;
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_DEBUG_ACTION0:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();
            part_switch->Type = (int)setting.FixedToGame(action.Value);
            thing->OnFixup();
            break;
        }

        case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_SELECTOR_OUTPUT:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();

            int index = MIN((int)setting.FixedToGame(action.Value), part_switch->Outputs.size() - 1);
            if (part_switch != NULL)
                part_switch->SelectorState = index;
            
            break;
        }
        case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();

            int num_inputs = MAX((int)setting.FixedToGame(action.Value), 1);
            if (part_switch != NULL)
                part_switch->SetInputCount(num_inputs);

            break;
        }
        case E_GOOEY_NETWORK_ACTION_AND_GATE_OUTPUT:
        case E_GOOEY_NETWORK_ACTION_OR_GATE_OUTPUT:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();
            if (part_switch != NULL)
                part_switch->OutputType = action.Value;
            break;
        }


        case E_GOOEY_NETWORK_ACTION_CHECKPOINT_MESH_STYLE:
        {
            DebugLog("checkpoint style uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetCheckpointStyle(thing, GetCheckpointType(thing), action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_CHECKPOINT_TYPE:
        {
            DebugLog("checkpoint type uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetCheckpointStyle(thing, action.Value, GetCheckpointStyle(thing));
            break;
        }
        case E_GOOEY_NETWORK_ACTION_CHECKPOINT_DELAY:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PCheckpoint* checkpoint = FindPartCheckpoint(thing);
            if (checkpoint == NULL) break;
            checkpoint->SpawnDelay = (u32)setting.FixedToGame(action.Value);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_LIGHTING:
        {
            world->ClearGlobalSettingUID(GLOBAL_LIGHT_SETTINGS);
            world->LightingFactor = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_LIGHT_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_DARKNESS:
        {
            world->ClearGlobalSettingUID(GLOBAL_LIGHT_SETTINGS);
            world->DarknessFactor = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_LIGHT_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_FOGGINESS:
        {
            world->ClearGlobalSettingUID(GLOBAL_LIGHT_SETTINGS);
            world->FogFactor = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_LIGHT_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_FOGCOLOUR:
        {
            world->ClearGlobalSettingUID(GLOBAL_LIGHT_SETTINGS);
            world->FogTintFactor = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_LIGHT_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_COLCORRECT:
        {
            world->ClearGlobalSettingUID(GLOBAL_LIGHT_SETTINGS);
            world->ColorCorrectionFactor = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_LIGHT_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_WATERLEVEL:
        {
            world->ClearGlobalSettingUID(GLOBAL_WATER_SETTINGS);
            world->WaterLevel = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_WATER_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_WATER_WAVE_MAGNITUDE:
        {
            world->ClearGlobalSettingUID(GLOBAL_WATER_WAVE_MAGNITUDE_SETTINGS);
            world->WaterWaveMagnitude = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_WATER_WAVE_MAGNITUDE_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_WATER_TINT:
        {
            world->ClearGlobalSettingUID(GLOBAL_WATER_TINT);
            world->WaterTint = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_WATER_TINT);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_WATER_MURKINESS:
        {
            world->ClearGlobalSettingUID(GLOBAL_WATER_MURKINESS);
            world->WaterMurkiness = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_WATER_MURKINESS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_WATER_BITS:
        {
            world->ClearGlobalSettingUID(GLOBAL_WATER_BITS);
            world->WaterBits = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_WATER_BITS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_GRAVITY:
        {
            world->ClearGlobalSettingUID(GLOBAL_GRAVITY_SETTINGS);
            world->Gravity = setting.FixedToGame(action.Value);
            world->ResetToDefaultGlobalSettings(GLOBAL_GRAVITY_SETTINGS);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_WIND_DIRECTION:
        {
            PEffector* effector = world->GetThing()->GetPEffector();
            if (effector == NULL) break;
            effector->WindDirection = setting.FixedToGame(action.Value);

            effector->PosVel = v2(
                effector->WindStrength * mmalex::cos(effector->WindDirection),
                effector->WindStrength * mmalex::sin(effector->WindDirection)
            );

            break;
        }

        case E_GOOEY_NETWORK_ACTION_WIND_VELOCITY:
        {
            PEffector* effector = world->GetThing()->GetPEffector();
            if (effector == NULL) break;
            effector->WindStrength = setting.FixedToGame(action.Value);

            effector->PosVel = v2(
                effector->WindStrength * mmalex::cos(effector->WindDirection),
                effector->WindStrength * mmalex::sin(effector->WindDirection)
            );

            break;
        }

        case E_GOOEY_NETWORK_ACTION_SHAPE_ANIMATION_SPEED:
        {
            float speed = setting.FixedToGame(action.Value);
            DebugLog("E_GOOEY_NETWORK_ACTION_SHAPE_ANIMATION_SPEED: %08x (%f)\n", (u32)action.Value, speed);

            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
                if (mesh != NULL)
                    mesh->TextureAnimationSpeed = speed;
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_MESH_ANIMATION_SPEED:
        {
            float speed = setting.FixedToGame(action.Value);
            DebugLog("E_GOOEY_NETWORK_ACTION_MESH_ANIMATION_SPEED: %08x (%f)\n", (u32)action.Value, speed);

            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PRenderMesh* mesh = thing->GetPRenderMesh();
                if (mesh != NULL)
                    mesh->AnimSpeed = speed;
            }

            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_AMMO_COUNT:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PScript* script = thing->GetPScript();
                if (script != NULL)
                    script->SetValue("BulletCount", action.Value);
            }

            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_EXPLOSIVE_STYLE:
        {
            DebugLog("explosive style uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetExplosiveStyle(thing, GetExplosiveType(thing), action.Value);
            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_LEVEL_KEY_STYLE:
        {
            DebugLog("level key style uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetLevelKeyStyle(thing, action.Value);
            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_MAGIC_EYE_STYLE:
        {
            DebugLog("level key style uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetMagicEyeStyle(thing, action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_MAGIC_EYE_AWARENESS_RADIUS:
        {
            float radius = setting.FixedToGame(action.Value);
            DebugLog("E_GOOEY_NETWORK_ACTION_MAGIC_EYE_AWARENESS_RADIUS: %08x (%f)\n", (u32)action.Value, radius);

            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PEnemy* enemy = thing->GetPEnemy();
                if (enemy != NULL)
                    enemy->Radius = radius;
            }

            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_LEVER_SWITCH_STYLE:
        {
            DebugLog("level key style uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetLeverSwitchStyle(thing, GetLeverSwitchType(thing), action.Value);
            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_BOUNCE_PAD_STYLE:
        {
            DebugLog("level key style uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetBouncePadStyle(thing, action.Value);
            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_SPIKE_PLATE_STYLE:
        {
            DebugLog("level key style uid=%d, index=%d\n", action.ThingUID, action.Value);
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetSpikePlateStyle(thing, GetSpikePlateType(thing), action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_INTERACTION_MODE:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PShape* shape = thing->GetPShape();
                if (shape != NULL)
                    shape->InteractPlayMode = action.Value;
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_BEVEL_TYPE:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetBevelType(thing, action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_BEVEL_SIZE:
        {
            float bevel_size = setting.FixedToGame(action.Value);

            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PShape* shape = thing->GetPShape();
                if (shape != NULL)
                    shape->BevelSize = bevel_size;
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_PHYSICS_TYPE:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                SetPhysicsType(thing, action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_PHYSICS_AUDIO:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PShape* shape = thing->GetPShape();
                if (shape != NULL)
                    shape->SoundEnumOverride = action.Value;
            }

            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_JUMP_MODIFIER:
        {
            float jump_modifier = setting.FixedToGame(action.Value);
            DebugLog("E_GOOEY_NETWORK_ACTION_JUMP_MODIFIER: %08x (%f)\n", (u32)action.Value, jump_modifier);

            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PScript* script = thing->GetPScript();
                if (script != NULL)
                    script->SetValue("JumpModifier", jump_modifier);
            }

            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_LAUNCHER_DISTANCE:
        {
            Launcher::SetDistance(world->GetThingByUID(action.ThingUID), setting.FixedToGame(action.Value));
            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_LAUNCHER_ANGLE:
        {
            Launcher::SetAngle(world->GetThingByUID(action.ThingUID), setting.FixedToGame(action.Value));
            break;
        }

        case E_GOOEY_NETWORK_ACTION_LAUNCHER_AUTO_ACTIVATED:
        {
            Launcher::SetAutoActivated(world->GetThingByUID(action.ThingUID), !action.Value);
            break;
        }
    
        case E_GOOEY_NETWORK_ACTION_SPEED_MODIFIER:
        {
            float speed_modifier = setting.FixedToGame(action.Value);
            DebugLog("E_GOOEY_NETWORK_ACTION_SPEED_MODIFIER: %08x (%f)\n", (u32)action.Value, speed_modifier);

            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PScript* script = thing->GetPScript();
                if (script != NULL)
                    script->SetValue("SpeedModifier", speed_modifier);
            }

            break;
        }
        
        case E_GOOEY_NETWORK_ACTION_ENEMY_COLLECTABLE:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PCreature* creature = thing->GetPCreature();
                if (creature != NULL)
                {
                    CThing* resource_thing = creature->ResourceThing.GetThing();
                    if (resource_thing != NULL)
                    {
                        SetCreatureBrainStyle(resource_thing, action.Value);
                    }
                }
            }
            break;
        }

        case E_GOOEY_NETWORK_ACTION_ENEMY_POINTS:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PScript* script = thing->GetPScript();
                if (script != NULL)
                    script->SetValue("PointsFixed", (s32)action.Value);
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_TUTORIAL_MODE:
        {
            world->IsTutorialLevel = (bool)action.Value;
            break;
        }



        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_LENGTH:
        {
            GET_PART(PJoint);
            float length = setting.FixedToGame(action.Value);
            if (!thing->Stamping || part->TweakTargetMinLength == 0.0f) part->Length = length;
            else 
            {
                part->TweakTargetMinLength = length;
                part->TweakTargetMaxLength = length;
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MIN_LENGTH:
        {
            GET_PART(PJoint);
            float length = setting.FixedToGame(action.Value);
            if (!thing->Stamping)
            {
                if (part->GetMaxLength() < length) part->AnimationRange = 0.0f;
                else 
                {
                    if (part->AnimationRange < 0.0f)
                    {
                        part->AnimationRange = length - part->Length;
                        break;
                    }
                    else
                    {
                        part->AnimationRange = (part->AnimationRange + part->Length) - length;
                    }
                }

                part->Length = length;
            }
            else 
            {
                part->TweakTargetMinLength = length;
                part->TweakTargetMaxLength = MAX(part->TweakTargetMaxLength, length);
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MAX_LENGTH:
        {
            GET_PART(PJoint);
            float length = setting.FixedToGame(action.Value);
            if (!thing->Stamping)
            {
                if (length < part->GetMinLength())
                {
                    part->AnimationRange = 0.0f;
                    part->Length = length;
                }
                else if (part->AnimationRange <= 0.0f)
                {
                    part->AnimationRange = ((float)part->Length + part->AnimationRange) - length;
                    part->Length = length;
                }
                else part->AnimationRange = length - (float)part->Length;
            }
            else
            {
                part->TweakTargetMaxLength = length;
                part->TweakTargetMinLength = MIN(length, part->TweakTargetMinLength);
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_NUMTURNS:
        {
            GET_PART(PJoint);
            float value = setting.FixedToGame(action.Value);
            if (part->AnimationRange < 0.0f) value = -value;
            part->AnimationRange = value;
            break;
        }
        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_TIME:
        {
            GET_PART(PJoint)->AnimationTime = MIN(setting.FixedToGame(action.Value), 200.0f);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PAUSE:
        {
            GET_PART(PJoint)->AnimationPause = setting.FixedToGame(action.Value);
            break;
        }
        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PHASE:
        {
            GET_PART(PJoint)->AnimationPhase = setting.FixedToGame(action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_ANGLE:
        {
            GET_PART(PJoint)->Angle = setting.FixedToGame(action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_SPEED:
        {
            GET_PART(PJoint);
            float value = setting.FixedToGame(action.Value);
            if (part->AnimationSpeed != 0.0f)
                part->Direction = 0.0f < part->AnimationSpeed;

            if (part->Direction == 0)
                value = -value;

            part->AnimationSpeed = value;

            if (value == 0.0f) break;
            part->Direction = 0.0f < value ? 1 : 0;

            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STRENGTH:
        {
            float value = SATURATE(setting.FixedToGame(action.Value));
            GET_PART(PJoint)->Strength = mmalex::powf(value, 3.0f);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STIFF:
        {
            GET_PART(PJoint)->Stiff = (bool)action.Value;
            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_DIRECTION:
        {
            GET_PART(PJoint);
            part->Direction = action.Value;
            part->AnimationSpeed = action.Value ? abs(part->AnimationSpeed) : -abs(part->AnimationSpeed);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_FLIPPER_PISTON:
        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_FLIPPER_WOBBLE:
        {
            GET_PART(PJoint);
            part->AnimationPattern = action.Value ? JOINT_PATTERN_FLIPPER : JOINT_PATTERN_WAVE;
            // ...


            break;

        }

        case E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_REVERSED:
        {
            // ...
            break;
        }

        case E_GOOEY_NETWORK_ACTION_JOINT_BEHAVIOUR:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
                thing->Behaviour = (s32)action.Value;
            break;
        }

        case E_GOOEY_NETWORK_ACTION_VISIBLE_IN_PLAY_MODE:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing != NULL)
            {
                PJoint* joint = thing->GetPJoint();
                if (joint != NULL) 
                    joint->HideInPlayMode = !action.Value;
                PEmitter* emitter = thing->GetPEmitter();
                if (emitter != NULL)
                    emitter->HideInPlayMode = !action.Value;
                PMicroChip* microchip = thing->GetPMicroChip();
                if (microchip != NULL)
                    microchip->HideInPlayMode = !action.Value;
            }

            break;
        }

        case E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_CIRCUIT:
        {
            GET_PART(PMicroChip)->SetCircuitBoardVisible(action.Value);
            break;
        }

        case E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_WIRES:
        {
            GET_PART(PMicroChip)->WiresVisible = !action.Value;
            break;
        }

        case E_GOOEY_NETWORK_ACTION_MICRO_CHIP_HORIZONTAL_CIRCUIT:
        {
            GET_PART(PMicroChip)->KeepVisualVertical = (bool)action.Value;
            break;
        }

        case E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LINEARVEL: { GET_PART(PEmitter)->LinearVel = setting.FixedToGame(action.Value); break; }
        case E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_ANGULARVEL: { GET_PART(PEmitter)->AngVel = setting.FixedToGame(action.Value); break; }
        // E_GOOEY_NETWORK_ACTION_EMITTER_IGNORE_PARENTS_VELOCITY
        case E_GOOEY_NETWORK_ACTION_EMITTER_BEHAVIOUR: { GET_PART(PEmitter)->GetThing()->Behaviour = action.Value; break; }
        case E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_FREQUENCY: { GET_PART(PEmitter)->Frequency = (u32)setting.FixedToGame(action.Value); break; }
        case E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LIFETIME: { GET_PART(PEmitter)->Lifetime = (u32)setting.FixedToGame(action.Value); break; }
        case E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_PHASE: { GET_PART(PEmitter)->Phase = (u32)setting.FixedToGame(action.Value); break; }
        case E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXNUM: 
        { 
            u32 max_emitted = (u32)setting.FixedToGame(action.Value);
            GET_PART(PEmitter)->MaxEmitted = max_emitted;
            if (max_emitted == 0) part->CurrentEmitted = 0;
            break; 
        }
        case E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXATONCENUM: { GET_PART(PEmitter)->MaxEmittedAtOnce = (u32)setting.FixedToGame(action.Value); break; }
        // E_GOOEY_NETWORK_ACTION_EMITTER_RECYCLE
        // E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_CREATE
        // E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_DESTROY

    }

#undef GET_PART
}

void SetupCarousel(ECarouselType type, CVector<CCarouselItem>& items)
{
    CIconConfig basic_icons(106001, 2, 4);

    switch (type)
    {
        case CAROUSEL_JOINT_BEHAVIOUR:
        {
            CIconConfig icon(107100, 2, 4);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"On/Off", v4(1.0f)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Forwards/Backwards", v4(1.0f)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Single Cycle", v4(1.0f)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(4), L"Speed Scale", v4(1.0f)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(5), L"Positional", v4(1.0f)));

            break;
        }

        case CAROUSEL_EMITTER_BEHAVIOUR:
        {
            CIconConfig icon(107100, 2, 4);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"On/Off", v4(1.0f)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Emit Once", v4(1.0f)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(4), L"Speed Scale", v4(1.0f)));

            break;
        }

        case CAROUSEL_MESH_STYLE:
        {
            CIconConfig icon(2321600356ul, 2, 4);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Wood", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Copper", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Cardboard", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Silver", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(4), L"Plastic", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(5), L"Chrome", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(6), L"Rubber", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(7), L"Gold", v4(1.0)));

            break;
        }

        case CAROUSEL_CHECKPOINT:
        {
            CIconConfig icon(E_KEY_CHECKPOINT_TYPE_SDF, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Entrance", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Single-Life Checkpoint", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Double-Life Checkpoint", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Infinite-Lives Checkpoint", v4(1.0)));

            break;
        }
        
        case CAROUSEL_SWITCH_VISIBILITY:
        {
            CIconConfig icon(119287, 1, 2);

            items.push_back(CCarouselItem(basic_icons.Texture, basic_icons.GetUV(0), L"No", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Switch Visible", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Switch and Cables Visible", v4(1.0)));

            break;
        }
        
        case CAROUSEL_EXPLOSIVE_STYLE:
        {
            CIconConfig icon(E_KEY_EXPLOSIVE_STYLE_SDF, 2, 4);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Burn", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Shock", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Freeze", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(4), L"Stun", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(5), L"Launch", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(6), L"Add", v4(1.0)));

            break;
        }

        case CAROUSEL_LEVEL_KEY_STYLE:
        {
            CIconConfig icon(3391025295ul, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Gemstones", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Brass", v4(1.0)));

            break;
        }
        
        case CAROUSEL_MAGIC_EYE_STYLE:
        {
            CIconConfig icon(3391025295ul, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Normal", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Cute", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Evil", v4(1.0)));

            break;
        }
        
        case CAROUSEL_INTERACTION_MODE:
        {
            CIconConfig icon(E_KEY_INTERACTION_MODE_SDF, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Not Interactable", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Can Select", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Can Edit", v4(1.0)));

            break;
        }
        
        case CAROUSEL_BEVEL_TYPE:
        {
            CIconConfig icon(E_KEY_BEVEL_TYPE_SDF, 4, 4);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Rounded 1", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Rounded 2", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(4), L"Metal", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(5), L"Metal Beam", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(6), L"Gold", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(7), L"Fluid", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(8), L"Sponge", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(9), L"Squidgy", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(10), L"Stitched 1", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(11), L"Stitched 2", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(12), L"Couch", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Soft", v4(1.0)));
            
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"garage plastic", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"no bevel", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"irobot", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Default Sponge Bevel", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Default Hard Rounded", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Extreme Border", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Fake Leather", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tin Can", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier1", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier2", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier3", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier4", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier5", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier6", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier7", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier8", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier9", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier10", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier11", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier12", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier13", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier14", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier15", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier16", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier17", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier18", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier19", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier20", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier21", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier22", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier23", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Tarsier24", v4(1.0)));


            break;
        }
        
        case CAROUSEL_PHYSICS_TYPE:
        {
            CIconConfig icon(E_KEY_PHYSICS_TYPE_SDF, 4, 4);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Light Fragile 1 (cardboard)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Light Fragile 2 (paper)", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(7), L"Light Basic (WIP)", v4(1.0, 0.0, 0.0, 1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(7), L"Mid Basic (wood)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(11), L"Heavy Basic (stone)", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(4), L"Light Grabbable (polystyrene)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(5), L"Mid Grabbable (sponge)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(5), L"Heavy Grabbable (clay)", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(6), L"High Traction (rubber)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(8), L"Mid Traction (WIP)", v4(1.0, 0.0, 0.0, 1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(8), L"Low Traction (glass)", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(10), L"Light Metal (WIP)", v4(1.0, 0.0, 0.0, 1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(10), L"Common Metal (metal)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(9), L"Precious Metal (gold)", v4(1.0)));
            
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(8), L"Slippery Aerogel (plastic)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Rough Aerogel (fluid)", v4(1.0)));
            
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(12), L"Down Buoyant (WIP)", v4(1.0, 0.0, 0.0, 1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(12), L"Neutral Buoyant (pink)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(13), L"Strong Buoyant (peach)", v4(1.0)));

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(14), L"Infinite Mass (dark matter)", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"No Mass (hologram)", v4(1.0)));
            
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Clay", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Cork", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Styrofoam", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Ice", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Poly no grab", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Light Wood", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Cloth Light", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Cloth Light Traction", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Cloth Heavy", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Cloth Heavy Traction", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Plastic", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Plastic Heavy", v4(1.0, 1.0, 0.0, 0.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(15), L"V Metal Light", v4(1.0, 1.0, 0.0, 0.0)));

            break;
        }
        
        case CAROUSEL_PHYSICS_AUDIO:
        {
            CIconConfig icon(E_KEY_PHYSICS_TYPE_SDF, 4, 4);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"No Override", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Stone", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Metal", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Wood", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Polystyrene", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Cloth", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Cardboard", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Sponge", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Rubber", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Creative", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Unused - Fire", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Unused - Ice", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Unused - Electricity", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Glass", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Sackboy", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Buoyant", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Slime", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Foil", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Wicker", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"African Drum", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Metal Grill", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Skateboard", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Sandpaper", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Cutlery", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Plastic", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Matchbox", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Unused - Glass Bottle", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Fruit", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Golfball", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Football", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Beachball", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Basketball", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Metal Light", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Dissolve", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Paintball", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Silence", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Metal Can", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Treasure", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Sand", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP2 - Metal Digital", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP2 - Bubblewrap", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP2 - Books", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP2 - Biscuit", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Foil Hard", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP3 - Soil", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP3 - Paper", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP3 - ClothWet", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP3 - Leather", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP3 - Floaty", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"LBP3 - Collectabell", v4(1.0)));

            break;
        }
    }
}

void AttachCarouselHooks()
{
    // We need to update the switch case that controls which
    // response is used for the message.
    MH_Poke32(0x0031ea4c, 0x2b9b0000 + (NUM_CAROUSEL_TYPES - 1));

    // Initialise the switch table with the offsets to the default case
    const int SWITCH_LABEL = 0x0031ea70;
    const int NOP_LABEL = 0x0031eaf0;
    const int LABEL_COUNT = 0x11;
    static s32 TABLE[NUM_CAROUSEL_TYPES];
    for (int i = 0; i < NUM_CAROUSEL_TYPES; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    TABLE[CAROUSEL_JOINT_BEHAVIOUR] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_EMITTER_BEHAVIOUR] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_MESH_STYLE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_CHECKPOINT] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_SWITCH_VISIBILITY] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_EXPLOSIVE_STYLE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_LEVEL_KEY_STYLE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_MAGIC_EYE_STYLE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_INTERACTION_MODE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_BEVEL_TYPE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_PHYSICS_TYPE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_PHYSICS_AUDIO] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x00929f10, (u32)TABLE);
}

void AttachGooeyNetworkHooks()
{
    // We need to update the switch case that controls which
    // response is used for the message.
    MH_Poke32(0x00437024, 0x2b800000 + (E_GOOEY_NETWORK_ACTION_COUNT - 1));

    // Initialise the switch table with the offsets to the default case
    const int SWITCH_LABEL = 0x00437048;
    const int NOP_LABEL = 0x004372a8;
    const int LABEL_COUNT = 0x66;
    static s32 TABLE[E_GOOEY_NETWORK_ACTION_COUNT];
    for (int i = 0; i < E_GOOEY_NETWORK_ACTION_COUNT; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }
    

    TABLE[E_GOOEY_NETWORK_ACTION_SHAPE_ANIMATION_SPEED] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SHAPE_ANIMATION_SPEED_OFF] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;

    TABLE[E_GOOEY_NETWORK_ACTION_CHECKPOINT_MESH_STYLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_CHECKPOINT_TYPE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_CHECKPOINT_DELAY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;

    TABLE[E_GOOEY_NETWORK_ACTION_TUTORIAL_MODE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;


    TABLE[E_GOOEY_NETWORK_ACTION_WIND_DIRECTION] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_WIND_VELOCITY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;


    TABLE[E_GOOEY_NETWORK_ACTION_GRAVITY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_LIGHTING] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_DARKNESS] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_FOGGINESS] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_FOGCOLOUR] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_COLCORRECT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;

    TABLE[E_GOOEY_NETWORK_ACTION_WATERLEVEL] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_WATER_WAVE_MAGNITUDE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_WATER_TINT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_WATER_MURKINESS] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_WATER_BITS] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    
    TABLE[E_GOOEY_NETWORK_ACTION_MESH_ANIMATION_SPEED] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_MESH_ANIMATION_SPEED_OFF] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_VISIBLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_COLOUR] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_BATTERY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_DEBUG_ACTION0] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_AND_GATE_OUTPUT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_OR_GATE_OUTPUT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_INVERTED] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_SELECTOR_OUTPUT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_ANGLE_RANGE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    
    TABLE[E_GOOEY_NETWORK_ACTION_EXPLOSIVE_STYLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_LEVEL_KEY_STYLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_MAGIC_EYE_STYLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_MAGIC_EYE_AWARENESS_RADIUS] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_LEVER_SWITCH_STYLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_BOUNCE_PAD_STYLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SPIKE_PLATE_STYLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_INTERACTION_MODE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    
    TABLE[E_GOOEY_NETWORK_ACTION_AMMO_COUNT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_LAUNCHER_DISTANCE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_LAUNCHER_ANGLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_LAUNCHER_AUTO_ACTIVATED] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_JUMP_MODIFIER] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SPEED_MODIFIER] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_ENEMY_COLLECTABLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_ENEMY_POINTS] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_BEVEL_TYPE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_BEVEL_SIZE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_PHYSICS_TYPE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_PHYSICS_AUDIO] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;

#ifdef __NEW_LOGIC_SYSTEM__
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_LENGTH] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MIN_LENGTH] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_MAX_LENGTH] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_NUMTURNS] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_TIME] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PAUSE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_PHASE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_ANGLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_SPEED] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STRENGTH] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_STIFF] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_FLIPPER] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_DIRECTION] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_REVERSED] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_TWEAK_VISIBLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_JOINT_BEHAVIOUR] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE; 
    TABLE[E_GOOEY_NETWORK_ACTION_VISIBLE_IN_PLAY_MODE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_CIRCUIT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_MICRO_CHIP_SHOW_WIRES] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_MICRO_CHIP_HORIZONTAL_CIRCUIT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;

    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LINEARVEL] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_ANGULARVEL] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_IGNORE_PARENTS_VELOCITY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_BEHAVIOUR] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_FREQUENCY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_LIFETIME] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_PHASE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXNUM] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_TWEAK_MAXATONCENUM] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
	TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_RECYCLE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
	TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_CREATE] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
	TABLE[E_GOOEY_NETWORK_ACTION_EMITTER_EFFECT_DESTROY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
#endif

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x00931de4, (u32)TABLE);
}

// fix for sliders in subframes
bool ParentAllowsChildrenInput(CGooeyNodeManager* manager, CGooeyNode* node, int input)
{
    if (node != NULL && manager->CurrentHighlightNode != node && node->Parent != NULL && node->Parent->IsType(2))
    {
        CGooeyNodeContainer* container = (CGooeyNodeContainer*)node->Parent;
        if (manager->CurrentHighlightNode == node->Parent)
        {
            if (input & container->DescendantAcceptedInput)
                return input & (container->AcceptedInput | container->AllowChildrenInput);
        }
        else if (input & container->DescendantAcceptedInput && input & (container->AcceptedInput | container->AllowChildrenInput))
            return ParentAllowsChildrenInput(manager, container, input);
    }

    return false;
}

void InitGooeyNetworkHooks()
{
    AttachGooeyNetworkHooks();
    AttachCarouselHooks();
    TweakSettingNativeFunctions::Register();

    // Replace custom page draw functions with >3000
    MH_Poke32(0x00384444, 0x409c01bc);
    MH_Poke32(0x00383d30, 0x409c0910);
    MH_Poke32(0x0037db30, 0x4198ffd4);

    MH_PokeBranch(0x0031f63c, &_gooey_carousel_sdf_hack_hook);
    // MH_Poke32(0x0031f670, LIS(4, 0xa14));
    MH_Poke32(0x0031f67c, 0x80810028 /* lwz %r4, 0x28(%r1)*/ );

    MH_PokeHook(0x002f8c10, ParentAllowsChildrenInput);
}