#ifndef PART_SWITCH_H
#define PART_SWITCH_H

#include <vector.h>
#include <refcount.h>
#include <MMString.h>
#include <ReflectionVisitable.h>

#include "ResourceDescriptor.h"
#include "Part.h"
#include "PlayerNumber.inl"

#include "hack_thingptr.h"

// The implementation of this file is mostly custom since I'm adding a more
// advanced switch system to LBP1, will try to make it somewhat mimick
// how it works in LBP2, but with some additional nonsense.


#define MAX_PORTS (100)

enum SwitchBehavior
{
    SWITCH_BEHAVIOR_OFF_ON,
    SWITCH_BEHAVIOR_SPEED_SCALE,
    SWITCH_BEHAVIOR_DIRECTION,
    SWITCH_BEHAVIOR_ONE_SHOT,
    SWITCH_BEHAVIOR_COUNT
};

enum SwitchEvent
{
    SWITCH_EVENT_UPDATE,
    SWITCH_EVENT_REGISTER,
    SWITCH_EVENT_UNREGISTER
};

enum ESwitchType
{
	SWITCH_TYPE_INVALID = -1,
	SWITCH_TYPE_BUTTON = 0,
	SWITCH_TYPE_LEVER = 1,
	SWITCH_TYPE_TRINARY = 2,
	SWITCH_TYPE_PROXIMITY = 3,
	SWITCH_TYPE_KEY = 4,
	SWITCH_TYPE_STICKER = 5,
	SWITCH_TYPE_GRAB = 6,
	SWITCH_TYPE_PRESSURE = 7,
	SWITCH_TYPE_PAINT = 8,
	SWITCH_TYPE_CONTROLLER_BINARY = 9,
	SWITCH_TYPE_CONTROLLER_ANALOG = 10,
	SWITCH_TYPE_SACKBOT_KEY = 11,
	SWITCH_TYPE_SACKBOT_PLAYER = 12,
	SWITCH_TYPE_SACKBOT_BOT = 13,
	SWITCH_TYPE_AND = 14,
	SWITCH_TYPE_COUNTDOWN = 15,
	SWITCH_TYPE_TIMER = 16,
	SWITCH_TYPE_TOGGLE = 17,
	SWITCH_TYPE_IMPACT = 18,
	SWITCH_TYPE_RANDOM = 19,
	SWITCH_TYPE_DIRECTION = 20,
	SWITCH_TYPE_OR = 21,
	SWITCH_TYPE_XOR = 22,
	SWITCH_TYPE_NOT = 23,
	SWITCH_TYPE_NOP = 24,
	SWITCH_TYPE_MOISTURE = 25,
	SWITCH_TYPE_INACTIVE = 26,
	SWITCH_TYPE_SIGN_SPLIT = 27,
	SWITCH_TYPE_ALWAYS_ON = 28,
	SWITCH_TYPE_ANIMATIC = 29,
	SWITCH_TYPE_SCORE = 30,
	SWITCH_TYPE_DEATH = 31,
	SWITCH_TYPE_CUTSCENE_CAM_FINISHED = 32,
	SWITCH_TYPE_CUTSCENE_CAM_ACTIVE = 33,
	SWITCH_TYPE_MAGIC_MOUTH = 34,
	SWITCH_TYPE_SELECTOR = 35,
	SWITCH_TYPE_MICROCHIP = 36,
	SWITCH_TYPE_CIRCUIT_BOARD = 37,
	SWITCH_TYPE_CONTROL_PAD = 38,
	SWITCH_TYPE_PROJECTILE = 39,
	SWITCH_TYPE_CIRCUIT_NODE = 40,
	SWITCH_TYPE_ANGLE = 41,
	SWITCH_TYPE_VELOCITY_LINEAR = 42,
	SWITCH_TYPE_VELOCITY_ANGULAR = 43,
	SWITCH_TYPE_MOVINATOR_PAD = 44,
	SWITCH_TYPE_MOTION_RECORDER = 45,
	SWITCH_TYPE_FILTER = 46,
	SWITCH_TYPE_COLOURGATE = 47,
	SWITCH_TYPE_WAVEGENERATOR = 48,
	SWITCH_TYPE_EMITTEE = 49,
	SWITCH_TYPE_POCKET_ITEM = 50,
	SWITCH_TYPE_POSE = 51,
	SWITCH_TYPE_ADVANCED_COUNTDOWN = 52,
	SWITCH_TYPE_STATE_SENSOR = 53,
	SWITCH_TYPE_KEY_REMOTE = 54,
	SWITCH_TYPE_VITA_PAD = 55,
	SWITCH_TYPE_TWEAK_TRIGGER = 56,
	SWITCH_TYPE_WORM_HOLE = 57,
	SWITCH_TYPE_QUEST = 58,
	SWITCH_TYPE_GRID_MOVER = 59,
	SWITCH_TYPE_QUEST_SENSOR = 60,
	SWITCH_TYPE_ADVENTURE_ITEM_GETTER = 61,
	SWITCH_TYPE_SHARDINATOR = 62,
	SWITCH_TYPE_POCKET_ITEM_DISPENSER_DEPRECATED = 63,
	SWITCH_TYPE_POCKET_ITEM_PEDESTAL = 64,
	SWITCH_TYPE_DATA_SAMPLER = 65,
	SWITCH_TYPE_TREASURE_SLOT = 66,
	SWITCH_TYPE_STREAMING_HINT = 67,
	SWITCH_TYPE_JOINT_POSITION = 68,
	SWITCH_TYPE_SLIDE = 69,
	SWITCH_TYPE_IN_OUT_MOVER = 70,
	SWITCH_TYPE_SAVE_CHIP = 71,
	SWITCH_TYPE_PLATFORM_SENSOR = 72,
	SWITCH_TYPE_RACE_END = 73,
	SWITCH_TYPE_ANTI_STREAMING = 74,
	SWITCH_TYPE_GAME_LIVE_STREAMING_CHOICE = 75,
	SWITCH_TYPE_SEARCHLIGHT = 76,
	SWITCH_TYPE_POPIT_CURSOR_SENSOR = 77,
	SWITCH_TYPE_PROGRESS_BOARD = 78,
	SWITCH_TYPE_BOUNCER = 79,
	SWITCH_TYPE_KILL_TWEAKER = 80,
	SWITCH_TYPE_POWERUP_TWEAKER = 81,
	SWITCH_TYPE_RACE_START = 82,
	SWITCH_TYPE_DECORATION_MOUNT = 83,
	SWITCH_TYPE_SPRING_SENSOR = 84,

    NUM_SWITCH_TYPES
};

class CSwitchDefinition {
    public:
        inline CSwitchDefinition() : BaseMesh(), InputPortMesh(), OutputPortMesh(), AuxillaryPortMesh()
        {
            SwitchType = SWITCH_TYPE_INVALID;
    
            NumOutputs = 1;
            NumInputs = 1;
            NumAuxillaryInputs = 0;
            
            MaxInputs = 1;
            MaxOutputs = 1;
    
            Left = 60.0f;
            Right = 100.0f;
            Top = 60.0f;
            Bottom = 60.0f;
    
            PortIncrementBias = 0.0f;
            InputPortRadius = 20.0f;
            OutputPortRadius = 20.0f;
            AuxillaryPortRadius = 20.0f;
        }
    public:
        inline v4 GetInputPortOffset(int index, int num_ports) const
        {
            float spacing = (Top + Bottom) / (num_ports + 1.0f);
            return v4(-Left, Top - (spacing * (index + 1)) - (InputPortRadius / 2.0f * index), 0.0f, 1.0f);
        }
    
        inline v4 GetOutputPortOffset(int index, int num_ports) const
        {
            float spacing = (Top + Bottom) / (num_ports + 1.0f);
            return v4(Right, Top - (spacing * (index + 1)) - (OutputPortRadius / 2.0f * index), 0.0f, 1.0f);
        }
    
        inline v4 GetAuxillaryPortOffset(int index, int num_ports) const
        {
            float spacing = (Left + Right) / (num_ports + 1.0f);
            return v4(Left - (spacing * (index + 1)) - (AuxillaryPortRadius / 2.0f * index), -Bottom, 0.0f, 1.0f);
        }
    public:
        CP<RMesh> BaseMesh;
        CP<RMesh> InputPortMesh;
        CP<RMesh> OutputPortMesh;
        CP<RMesh> AuxillaryPortMesh;
    
        ESwitchType SwitchType;
    
        u32 NumOutputs;
        u32 NumInputs;
        u32 NumAuxillaryInputs;
    
        u32 MaxInputs;
        u32 MaxOutputs;
    
        f32 Left;
        f32 Right;
        f32 Bottom;
        f32 Top;
    
        f32 PortIncrementBias;
        f32 InputPortRadius;
        f32 OutputPortRadius;
        f32 AuxillaryPortRadius;
    };

class ConnectorPointList {
public:
    inline ConnectorPointList() : Points(), Initialized() {}
public:
    v2 Points[8];
    bool Initialized;
};

class CSwitchSignal {
public:
    inline CSwitchSignal() :
    Analogue(0.0f), Ternary(0), Player(E_PLAYER_NUMBER_NONE)
    {}

    inline CSwitchSignal(float analogue) : Analogue(analogue), Ternary(0), Player(E_PLAYER_NUMBER_NONE)
    {}
public:
    float Analogue;
    int Ternary;
    int Player;
};

class CSwitchTarget {
public:
    inline CSwitchTarget() : Thing(), Port()
    {}
    
    inline CSwitchTarget(const CThingPtr& target, int port) : Thing(target), Port(port) 
    {}

    inline CSwitchTarget(const CSwitchTarget& target)
    {
        *this = target;
    }

    inline CSwitchTarget& operator=(CSwitchTarget const& rhs) 
    { 
        Thing = rhs.Thing;
        Port = rhs.Port;
        return *this;
    }
public:
    CThingPtr Thing;
    int Port;
};

class PSwitch;
class CSwitchOutput : public CReflectionVisitable {
public:
    inline CSwitchOutput() : CReflectionVisitable(), Activation(), TargetList(), Owner(), Port(), UserDefinedName()
    {}

    ~CSwitchOutput();
public:
    bool AddTarget(CThing* thing, int port);
    bool RemoveTarget(CThing* thing, int port);
    int GetTargetIndex(CThing* thing, int port);
public:
    CSwitchSignal Activation;
    CVector<CSwitchTarget> TargetList;
    PSwitch* Owner;
    int Port;
    const wchar_t* UserDefinedName;
};

class SPortData {
public:
    inline SPortData() : From(), To() {}
    inline SPortData(u8 from, u8 to) : From(from), To(to) {}
public:
    u8 From;
    u8 To;
};

class PSwitch : public CPart {
public:
    void InitializeExtraData();
    void DestroyExtraData();
    void OnPartLoaded();
    bool HasCustomData();
    void GenerateLegacyData();
    void ClearLegacyData();
    const CSwitchDefinition& GetSwitchDefinition() const;
public:
    void Update();
    CSwitchSignal GetActivationFromInput(int port);
    CSwitchSignal GetNewActivation(int port);
public:
    bool RaycastConnector(v4 start, v4 dir, float& t, CThing*& hit);
public:
    CVector<CThingPtr> TargetList;
    CVector<CThingPtr> EditorHackTargetList;
    CVector<v4> ConnectorPos;
    CVector<bool> ConnectorGrabbed;
    CVector<ConnectorPointList> ConnectorPoints;
    CResourceDescriptor<RPlan> RefSticker;
    CThingPtr ReferenceThing;
    v4 PortPosOffset;
    v4 LooseConnectorPos;
    v4 LooseConnectorBaseOffset;
    bool LooseConnectorGrabbed;
    ConnectorPointList LooseConnectorPoints;
    bool HideConnectors;
    bool Initialised;
    bool Registered;
    bool Inverted;
    float Radius;
    int ColorIndex;
    int BehaviourOld;
    int Type;
    bool HideInPlayMode;
    bool RequireAll;
    float DeprecatedManualActivation;
    float PlatformVisualFactor;
    float OldActivation;
    float ActivationHoldTime;
    union
    {
        int BulletsRequired;
        int NumInputs;
    };
    int BulletsDetected;
    int BulletPlayerNumber;
    u32 BulletRefreshTime;
    float Activation;
    float unk1;
    float unk2;
    bool ValueChange;
    float LightActivation;
    float AngleRange;
    int LayerType;
private:
    char Pad[0xc];
public:
    int UpdateFrame;
    int Behaviour;
    int OutputType;
    CSwitchSignal ManualActivation;
    CVector<CSwitchOutput*> Outputs;
    CVector<SPortData> PortData;
};

#endif // PART_SWITCH_H