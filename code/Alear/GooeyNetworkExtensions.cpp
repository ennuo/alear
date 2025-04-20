#include "AlearHooks.h"
#include "CheckpointStyles.h"

#include <GooeyNetworkAction.h>
#include <PlayerNumber.inl>
#include <hook.h>
#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <ResourceGFXTexture.h>
#include <ResourceScript.h>
#include <ResourceTranslationTable.h>
#include <ResourceSystem.h>
#include <PartPhysicsWorld.h>
#include <thing.h>
#include <mmalex.h>

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
        Suffix = MakeLamsKeyID(lams_key, NULL);
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
        ToolTip = MakeLamsKeyID(lams_key, NULL);
        return *this;
    }

    CTweakSetting& SetToolTip(u32 lams_key)
    {
        ToolTip = lams_key;
        return *this;
    }

    CTweakSetting& SetIcon(CIconConfig& config, u32 index)
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
        StepSize = 1.0f;
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

        switch (network_action)
        {
            case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_INVERTED: return thing->GetPSwitch()->Inverted;
            case E_GOOEY_NETWORK_ACTION_SWITCH_VISIBILITY:
            {
                PSwitch* part_switch = thing->GetPSwitch();
                if (part_switch->HideInPlayMode == true && part_switch->HideConnectors == true) return 0;
                if (part_switch->HideInPlayMode == false && part_switch->HideConnectors == true) return 1;
                return 2;
            }
            case E_GOOEY_NETWORK_ACTION_SWITCH_BATTERY: return setting.GameToFixed(thing->GetPSwitch()->ManualActivation.Analogue);
            case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT: return setting.GameToFixed(thing->GetPSwitch()->NumInputs);
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
        }

        return 0;
    }

    f32 GetStylesheetScalingFactor(CScriptObjectGooey* gooey)
    {
        if (gooey == NULL) return 0.0f;
        CScriptedGooeyWrapper* wrapper = gooey->GetNativeObject();
        if (wrapper == NULL) return 0.0f;
        return wrapper->Manager->StylesheetScalingFactor;
    }

    void Register()
    {
        RegisterNativeFunction("TweakSetting", "GetTweakSetting__i", true, NVirtualMachine::CNativeFunction1<CScriptObjectInstance*, EGooeyNetworkAction>::Call<Get>);
        RegisterNativeFunction("TweakSetting", "GetFixedTweakData__ii", true, NVirtualMachine::CNativeFunction2<u32, EGooeyNetworkAction, u32>::Call<GetFixedTweakData>);
        RegisterNativeFunction("Gooey", "GetStylesheetScalingFactor__", false, NVirtualMachine::CNativeFunction1<f32, CScriptObjectGooey*>::Call<GetStylesheetScalingFactor>);
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
    CIconConfig visualstyle_texture(2172845213ul, 1, 1);
    CIconConfig inputs_icon_texture(2190825069ul, 1, 1);
    CIconConfig batteryoutput_icon_texture(2216320518ul, 1, 1);
    CIconConfig visibleinplaymode_icon_texture(2958198268ul, 1, 1);
    CIconConfig sensor_icons(2481301814ul, 4, 4);

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
        .SetMinMax(0.0f, 200.0f)
        .SetDebugToolTip(L"Animation Speed");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_CHECKPOINT_MESH_STYLE)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(visualstyle_texture, 0)
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

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_VISIBILITY)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(visibleinplaymode_icon_texture, 0)
        .SetIcon(CAROUSEL_SWITCH_VISIBILITY)
        .SetDebugToolTip(L"Electronics and Cable Visibility");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_BATTERY)
        .SetupFraction()
        .SetIcon(batteryoutput_icon_texture, 0)
        .SetMinMax(-100.0f, 100.0f)
        .SetDebugToolTip(L"Output");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT)
        .SetupCounter()
        .SetIcon(inputs_icon_texture, 0)
        .SetMinMax(2.0f, 100.0f)
        .SetSteps(1.0f, 1.0f)
        .SetDebugToolTip(L"Number of Ports");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_AND_GATE_OUTPUT)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(sensor_icons, 9)
        .SetIcon(CAROUSEL_AND_GATE_OUTPUT)
        .SetDebugToolTip(L"Output Value");

    GetTweakSetting(E_GOOEY_NETWORK_ACTION_OR_GATE_OUTPUT)
        .SetWidget(TWEAK_WIDGET_CAROUSEL)
        .SetIcon(sensor_icons, 9)
        .SetIcon(CAROUSEL_OR_GATE_OUTPUT)
        .SetDebugToolTip(L"Output Value");

    

    

    

    // visual style
        // Entrance
        // Infinite-Life Checkpoint
        // Double-Life Checkpoint
        // Checkpoint
        // Visual Style : Plastic
        //              : Chrome
        //              : Cardboard
        //              : Wood



    return true;
}

void DoNetworkActionResponse(CMessageGooeyAction& action)
{
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;

    CTweakSetting& setting = GetTweakSetting(action.Action);

    switch (action.Action)
    {
        case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_INVERTED:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();
            if (part_switch != NULL) part_switch->Inverted = action.Value;
            break;
        }

        case E_GOOEY_NETWORK_ACTION_SWITCH_VISIBILITY:
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
        case E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT:
        {
            CThing* thing = world->GetThingByUID(action.ThingUID);
            if (thing == NULL) break;
            PSwitch* part_switch = thing->GetPSwitch();

            int num_inputs = MAX((int)setting.FixedToGame(action.Value), 2);
            if (part_switch != NULL)
            {
                for (int i = part_switch->NumInputs - 1; i >= num_inputs; --i)
                {
                    CSwitchOutput* input = thing->GetInput(i);
                    if (input != NULL)
                        input->RemoveTarget(thing, i);
                }

                part_switch->NumInputs = num_inputs;
            }

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

        case E_GOOEY_NETWORK_ACTION_TUTORIAL_MODE:
        {
            world->IsTutorialLevel = (bool)action.Value;
            break;
        }
    }
}

void SetupCarousel(ECarouselType type, CVector<CCarouselItem>& items)
{
    CIconConfig basic_icons(2173444119ul, 2, 4);

    switch (type)
    {
        case CAROUSEL_MESH_STYLE:
        {
            CIconConfig icon(2321600356ul, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Cardboard", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Wood", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Plastic", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Chrome", v4(1.0)));

            break;
        }

        case CAROUSEL_CHECKPOINT:
        {
            CIconConfig icon(3926674768ul, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Entrance", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Single-Life Checkpoint", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Double-Life Checkpoint", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Infinite-Lives Checkpoint", v4(1.0)));

            break;
        }

        case CAROUSEL_SWITCH_VISIBILITY:
        {
            CIconConfig icon(3442365560ul, 1, 2);

            items.push_back(CCarouselItem(basic_icons.Texture, basic_icons.GetUV(0), L"No", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Switch Visible", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Switch and Cables Visible", v4(1.0)));

            break;
        }

        case CAROUSEL_AND_GATE_OUTPUT:
        {
            CIconConfig icon(3778299051ul, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(0), L"Minimum Input", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(2), L"Multiply Inputs", v4(1.0)));

            break;
        }

        case CAROUSEL_OR_GATE_OUTPUT:
        {
            CIconConfig icon(3778299051ul, 2, 2);

            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(1), L"Max Input", v4(1.0)));
            items.push_back(CCarouselItem(icon.Texture, icon.GetUV(3), L"Add Inputs", v4(1.0)));

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

    TABLE[CAROUSEL_MESH_STYLE] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_SWITCH_VISIBILITY] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_AND_GATE_OUTPUT] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_OR_GATE_OUTPUT] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;
    TABLE[CAROUSEL_CHECKPOINT] = (u32)&_gooey_carousel_type_hook - (u32)TABLE;

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


    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_VISIBILITY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_COLOUR] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_BATTERY] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_PORT_COUNT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_AND_GATE_OUTPUT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_OR_GATE_OUTPUT] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;
    TABLE[E_GOOEY_NETWORK_ACTION_SWITCH_TWEAK_INVERTED] = (u32)&_custom_gooey_network_action_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x00931de4, (u32)TABLE);
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


}