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


MH_DefineFunc(PSwitch_RaycastConnector, 0x0004d764, TOC0, bool, PSwitch*, v4, v4, float&, CThing*&);
bool PSwitch::RaycastConnector(v4 start, v4 dir, float& t, CThing*& hit)
{
    return PSwitch_RaycastConnector(this, start, dir, t, hit);
}

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

void PSwitch::InitializeExtraData()
{
    new (&Outputs) CVector<CSwitchOutput>();
    Behaviour = 0;
    UpdateFrame = 0;
}

void PSwitch::DestroyExtraData()
{
    Outputs.~CVector();
}

void CustomRaycastAgainstSwitches(CPoppet* poppet)
{
    poppet->m_bestTFromPSwitches = 1.0e+20f;
    poppet->m_havePSwitchHit = false;

    if (poppet->GetSubMode() != SUBMODE_NONE) return;

    PWorld* world = gGame->GetWorld();
    CThing* ignored = poppet->GetThingToIgnore();

    CRaycastResults& raycast = poppet->m_raycastResultFromPSwitches;
    raycast.HitPort = -1;
    raycast.RefPort = -1;

    float t;
    CThing* hit;

    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* sw = (*it);
        CThing* thing = sw->GetThing();

        if (thing == ignored) continue;

        if (sw->Type == SWITCH_TYPE_AND)
        {
            CSwitchDefinition& def = gSwitchDefinitions[sw->Type];

            m44& wpos = thing->GetPPos()->Game.WorldPosition;


            v4 input_offsets[] =
            {
                wpos * def.GetInputPortOffset(0, def.NumInputs),
                wpos * def.GetInputPortOffset(1, def.NumInputs)
            };

            v4 output_offsets[] =
            {
                wpos * def.GetOutputPortOffset(0, def.NumOutputs)
            };

            for (int i = 0; i < def.NumOutputs; ++i)
            {
                v4& port_offset = output_offsets[i];
                if (!NPoppetUtils::RaySphereIntersect(port_offset, def.OutputPortRadius, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
                if (t >= 1.0e+20f) continue;

                DebugLog("hitting output port %d on AND gate!!!\n", i);

                raycast.SwitchConnector = true;
                raycast.HitThing = thing;
                raycast.RefThing = thing;

                poppet->m_bestTFromPSwitches = t;
                poppet->m_havePSwitchHit = true;

                raycast.BaryU = 0.0f;
                raycast.BaryV = 0.0f;
                raycast.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
                raycast.TriIndex = 0;
                raycast.OnCostumePiece = -1;
                raycast.DecorationIdx = -1;

                raycast.HitPort = i;
            }

            for (int i = 0; i < def.NumInputs; ++i)
            {
                v4& port_offset = input_offsets[i];
                if (!NPoppetUtils::RaySphereIntersect(port_offset, def.InputPortRadius, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
                if (t >= 1.0e+20f) continue;

                DebugLog("hitting input port %d on AND gate!!!\n", i);

                raycast.SwitchConnector = false;
                raycast.HitThing = thing;
                raycast.RefThing = thing;
                
                poppet->m_bestTFromPSwitches = t;
                poppet->m_havePSwitchHit = true;

                raycast.BaryU = 0.0f;
                raycast.BaryV = 0.0f;
                raycast.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
                raycast.TriIndex = 0;
                raycast.OnCostumePiece = -1;
                raycast.DecorationIdx = -1;

                raycast.RefPort = i;
            }

            // input ports are a sphere with a radius of 20
                // at x = -60
                // y = -35 / 35

            continue;
        }

        if (!sw->RaycastConnector(poppet->m_rayStart, poppet->m_rayDir, t, hit)) continue;
        if (t >= 1.0e+20f) continue;

        raycast.SwitchConnector = true;

        raycast.HitThing = hit;
        raycast.RefThing = thing;

        poppet->m_bestTFromPSwitches = t;
        poppet->m_havePSwitchHit = true;

        raycast.BaryU = 0.0f;
        raycast.BaryV = 0.0f;
        raycast.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
        raycast.TriIndex = 0;
        raycast.OnCostumePiece = -1;
        raycast.DecorationIdx = -1;

        raycast.HitPort = 0;
        raycast.RefPort = 0;
    }
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

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CRaycastResults& d);

#undef ADD

ReflectReturn GatherPoppetRaycastVariables(CGatherVariables& r, CRaycastResults& d)
{
    return Reflect(r, d);
}

extern "C" uintptr_t _raycast_hook;
extern "C" uintptr_t _popit_destroy_extra_data_hook;
extern "C" uintptr_t _popit_init_extra_data_hook;

void InitLogicSystemHooks()
{
    InitializeSwitchDefinitions();

    MH_InitHook((void*)0x00353370, (void*)&CustomRaycastAgainstSwitches);
    MH_InitHook((void*)0x007716e8, (void*)&GatherPoppetRaycastVariables);

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