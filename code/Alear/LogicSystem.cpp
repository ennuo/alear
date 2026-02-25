#include <PartSwitch.h>
#include <GfxPool.h>
#include <refcount.h>
#include <PartPhysicsWorld.h>
#include <ResourceGame.h>
#include <ResourceGfxMaterial.h>
#include <ResourceGFXMesh.h>
#include <ResourceDescriptor.h>
#include <ResourcePointer.h>
#include <Poppet.h>
#include <thing.h>
#include <ResourceGame.h>
#include <PartPhysicsWorld.h>
#include <hook.h>
#include <Variable.h>
#include <cell/DebugLog.h>

namespace NPoppetUtils
{
    MH_DefineFunc(RaySphereIntersect, 0x0038eedc, TOC1, bool, v4 center, float radius, v4 ray_pos, v4 ray_dir, float& intersect_dist);
}

namespace SystemFunctions
{
    MH_DefineFunc(DrawRectangle, 0x0046369c, TOC1, void, v4 min, v4 max, v4 col, bool world_space, bool z_test, bool fill);
}

// probably just going to load these from files
class CSwitchDefinition {
public:
    inline CSwitchDefinition() : BaseMesh(), InputPortMesh(), OutputPortMesh(), AuxillaryPortMesh()
    {
        SwitchType = SWITCH_TYPE_INVALID;

        NumOutputs = 1;
        NumInputs = 2;
        NumAuxillaryInputs = 0;

        MaxInputs = MAX_PORTS;
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

CSwitchDefinition gSwitchDefinitions[NUM_SWITCH_TYPES];
void InitializeSwitchDefinitions()
{
    DebugLog("initializing switch definitions...\n");
    DebugLog("adding baked AND gate...\n");
    
    CSwitchDefinition& def = gSwitchDefinitions[SWITCH_TYPE_AND];
    def.SwitchType = SWITCH_TYPE_AND;

    DebugLog("testing port offset calculations...\n");

    for (int i = 0; i < def.NumInputs; ++i)
    {
        v4 local_port_offset = def.GetInputPortOffset(i, def.NumInputs);
        DebugLog("ANDGate_InputPort%d: v3(%f, %f, %f)\n", i, local_port_offset.getX().getAsFloat(), local_port_offset.getY().getAsFloat(), local_port_offset.getZ().getAsFloat());
    }

    for (int i = 0; i < def.NumOutputs; ++i)
    {
        v4 local_port_offset = def.GetOutputPortOffset(i, def.NumOutputs);
        DebugLog("ANDGate_OutputPort%d: v3(%f, %f, %f)\n", i, local_port_offset.getX().getAsFloat(), local_port_offset.getY().getAsFloat(), local_port_offset.getZ().getAsFloat());
    }
}

void RenderSwitchDebug()
{
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;
    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* sw = (*it);
        if (sw == NULL || sw->Type != SWITCH_TYPE_AND) continue;

        CThing* thing = sw->GetThing();
        CSwitchDefinition& def = gSwitchDefinitions[sw->Type];
        m44& wpos = thing->GetPPos()->Game.WorldPosition;

        for (int i = 0; i < def.NumInputs; ++i)
        {
            v4 offset = def.GetInputPortOffset(i, def.NumInputs);
            v4 radius = v4(def.InputPortRadius, def.InputPortRadius, 0.0f, 0.0f);

            SystemFunctions::DrawRectangle(wpos * (offset - radius), wpos * (offset + radius), v4(1.0f), true, true, true);
        }

        for (int i = 0; i < def.NumOutputs; ++i)
        {
            v4 offset = def.GetOutputPortOffset(i, def.NumOutputs);
            v4 radius = v4(def.OutputPortRadius, def.OutputPortRadius, 0.0f, 0.0f);

            SystemFunctions::DrawRectangle(wpos * (offset - radius), wpos * (offset + radius), v4(1.0f), true, false, true);
        }
    }
}


// MH_DefineFunc(PSwitch_RaycastConnector, 0x0004d764, TOC0, bool, PSwitch*, v4, v4, float&, CThing*&);
// bool PSwitch::RaycastConnector(v4 start, v4 dir, float& t, CThing*& hit)
// {
//     return PSwitch_RaycastConnector(this, start, dir, t, hit);
// }

void CPoppet::InitializeExtraData()
{
    new (&HiddenList) CVector<CThingPtr>();
    StampMode = STAMP_DEFAULT;
    CustomPoppetOffset = v2(0.0f);
    CustomPoppetSize = v2(512.0f, 544.0f);
    ShowTether = true;
    HidePoppetGooey = false;
}

void CPoppet::DestroyExtraData()
{
    HiddenList.~CVector();
}

CSwitchOutput* CThing::GetInput(int port) const
{
    if (port < 0 || CustomThingData->InputList.size() <= port) return NULL;
    return CustomThingData->InputList[port];
}

void CThing::SetInput(CSwitchOutput* input, int port)
{
    CVector<CSwitchOutput*>& inputs = CustomThingData->InputList;
    if (inputs.size() <= port)
        inputs.try_resize(port + 1);
    inputs[port] = input;
}

void CThing::RemoveInput(int port)
{
    CSwitchOutput* input = GetInput(port);
    if (input != NULL)
        input->RemoveTarget(this, port);
    
    for (int i = port + 1; i < CustomThingData->InputList.size(); ++i)
    {
        input = GetInput(i);
        if (input != NULL)
        {
            input->RemoveTarget(this, port);
            input->AddTarget(this, port - 1);
        }
    }
}

int CSwitchOutput::GetTargetIndex(CThing* thing, int port)
{
    for (int i = 0; i < TargetList.size(); ++i)
    {
        CSwitchTarget& target = TargetList[i];
        if (target.Thing == thing && target.Port == port)
            return i;
    }

    return -1;
}

bool CSwitchOutput::AddTarget(CThing* thing, int port)
{
    if (thing == NULL) return false;

    CSwitchOutput* input = thing->GetInput(port);
    if (input != NULL)
        input->RemoveTarget(thing, port);

    TargetList.push_back(CSwitchTarget(thing, port));
    thing->SetInput(this, port);

    return true;
}

bool CSwitchOutput::RemoveTarget(CThing* thing, int port)
{
    if (thing == NULL) return false;

    int index = GetTargetIndex(thing, port);
    if (index == -1) return false;

    thing->SetInput(NULL, port);
    TargetList.erase(TargetList.begin() + index);

    return true;
}

CSwitchOutput::~CSwitchOutput()
{
    for (CSwitchTarget* target = TargetList.begin(); target != TargetList.end(); ++target)
        target->Thing->SetInput(NULL, target->Port);
}

void PSwitch::InitializeExtraData()
{
    new (&Outputs) CVector<CSwitchOutput>();
    new (&ManualActivation) CSwitchSignal();
    Behaviour = 0;
    OutputType = 0;
    CrappyOldLBP1Switch = true;
    PlaySwitchAudio = true;
}

void PSwitch::DestroyExtraData()
{
    for (int i = 0; i < Outputs.size(); ++i)
        delete Outputs[i];
    
    Outputs.~CVector();
}

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;
template<typename R>
ReflectReturn Reflect(R& r, CRaycastResults& d)
{
    ReflectReturn ret;
    ADD(HitPoint);
    ADD(Normal);
    ADD(BaryU);
    ADD(BaryV);
    ADD(TriIndex);
    ADD(HitThing);
    ADD(RefThing);
    ADD(OnCostumePiece);
    ADD(DecorationIdx);
    ADD(SwitchConnector);
    ADD(HitPort);
    ADD(RefPort);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CSwitchSignal& d)
{
    ReflectReturn ret;
    ADD(Analogue);
    ADD(Ternary);
    ADD(Player);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CSwitchTarget& d)
{
    ReflectReturn ret;
    ADD(Thing);
    ADD(Port);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CSwitchOutput& d)
{
    ReflectReturn ret;
    ADD(Activation);
    ADD(TargetList);
    return ret;
}

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CRaycastResults& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSwitchSignal& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSwitchTarget& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSwitchOutput& d);

template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, CSwitchSignal& d);
template ReflectReturn Reflect<CReflectionSaveVector>(CReflectionSaveVector& r, CSwitchSignal& d);

ReflectReturn GatherSwitchVariables(CGatherVariables& r, PSwitch& d)
{
    ReflectReturn ret;
    ADD(Inverted);
    ADD(Radius);
    ADD(ColorIndex);
    ADD(CrappyOldLBP1Switch);
    ADD(BehaviourOld);
    ADD(Outputs);
    ADD(HideInPlayMode);
    ADD(Type);
    ADD(ReferenceThing);
    ADD(ManualActivation);
    ADD(ActivationHoldTime);
    ADD(RequireAll);
    ADD(BulletsRequired);
    ADD(BulletsDetected);
    ADD(BulletRefreshTime);
    ADD(ResetWhenFull);
    ADD(Behaviour);
    ADD(HideConnectors);
    ADD(DetectUnspawnedPlayers);
    ADD(PlaySwitchAudio);
    return ret;
}

#undef ADD


ReflectReturn GatherPoppetRaycastVariables(CGatherVariables& r, CRaycastResults& d)
{
    return Reflect(r, d);
}

extern "C" uintptr_t _raycast_hook;
extern "C" uintptr_t _popit_destroy_extra_data_hook;
extern "C" uintptr_t _popit_init_extra_data_hook;

void OnPartSwitchDestructor(PSwitch* sw)
{
    sw->DestroyExtraData();
}

void InitLogicSystemHooks()
{
    MH_InitHook((void*)0x007716e8, (void*)&GatherPoppetRaycastVariables);

    MH_PokeCall(0x0004632c, OnPartSwitchDestructor);

    // replace stb with stw for collating raycast so it copies additional fields
    MH_Poke32(0x00356aa0, 0x815f004c /* lwz %r10, 0x4c(%r31) */);
    MH_Poke32(0x00356acc, 0x915e18fc /* stw %r10, 0x18fc(%r30) */);

    // raycast hack for local players
    MH_PokeBranch(0x0011067c, &_raycast_hook);

    // Increase the allocation size for the poppet class instance
    MH_Poke32(0x0073b5dc, LI(4, sizeof(CPoppet)));
    MH_Poke32(0x00231bdc, LI(4, sizeof(CPoppet)));
    MH_Poke32(0x000316bc, LI(4, sizeof(CPoppet)));

    // Add hooks for our custom data construction/destruction
    MH_PokeBranch(0x0034f264, &_popit_init_extra_data_hook);
    MH_PokeBranch(0x0034b4f4, &_popit_destroy_extra_data_hook);
}