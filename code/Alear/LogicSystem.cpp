#include "Hermite.h"

#include <PartSwitch.h>
#include <GfxPool.h>
#include <refcount.h>
#include <PartPhysicsWorld.h>
#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <ResourceGfxMaterial.h>
#include <ResourceGFXMesh.h>
#include <ResourceSystem.h>
#include <ResourceDescriptor.h>
#include <ResourceTranslationTable.h>
#include <ResourcePointer.h>
#include <Poppet.h>
#include <thing.h>
#include <ResourceGame.h>
#include <PartPhysicsWorld.h>
#include <hook.h>
#include <Variable.h>
#include <cell/DebugLog.h>

#include <cell/gcm.h>
#include <gfxcore.h>

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

const v4 PORT_ZBIAS = v4(0.0f, 0.0f, 10.0f, 0.0f);

namespace NPoppetUtils
{
    MH_DefineFunc(RaySphereIntersect, 0x0038eedc, TOC1, bool, v4 center, float radius, v4 ray_pos, v4 ray_dir, float& intersect_dist);
}

namespace SystemFunctions
{
    MH_DefineFunc(DrawRectangle, 0x0046369c, TOC1, void, v4 min, v4 max, v4 col, bool world_space, bool z_test, bool fill);
}

CSwitchDefinition gSwitchDefinitions[NUM_SWITCH_TYPES];
CSwitchDefinition gDefaultSwitchDefinition;

void InitializeSwitchDefinitions()
{
    DebugLog("initializing switch definitions...\n");

    {
        CSwitchDefinition& def = gSwitchDefinitions[SWITCH_TYPE_AND];
        def.SwitchType = SWITCH_TYPE_AND;
        def.MaxInputs = MAX_PORTS;
        def.NumInputs = 2;
    }

    {
        CSwitchDefinition& def = gSwitchDefinitions[SWITCH_TYPE_OR];
        def.SwitchType = SWITCH_TYPE_OR;
        def.MaxInputs = MAX_PORTS;
        def.NumInputs = 2;
    }

    {
        CSwitchDefinition& def = gSwitchDefinitions[SWITCH_TYPE_ALWAYS_ON];
        def.SwitchType = SWITCH_TYPE_ALWAYS_ON;
        def.MaxInputs = 0;
        def.NumInputs = 0;
    }

    {
        CSwitchDefinition& def = gSwitchDefinitions[SWITCH_TYPE_NOT];
        def.SwitchType = SWITCH_TYPE_NOT;
        def.MaxInputs = 1;
        def.NumInputs = 1;    
    }
}

void RenderSwitchDebug()
{
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;
    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* sw = (*it);
        if (sw == NULL) continue;

        CThing* thing = sw->GetThing();
        const CSwitchDefinition& def = sw->GetSwitchDefinition();
        m44& wpos = thing->GetPPos()->Game.WorldPosition;

        int num_inputs = def.NumInputs;
        if (sw->Type == SWITCH_TYPE_AND || sw->Type == SWITCH_TYPE_OR || sw->Type == SWITCH_TYPE_XOR || sw->Type == SWITCH_TYPE_SELECTOR)
            num_inputs = sw->NumInputs;

        for (int i = 0; i < num_inputs; ++i)
        {
            v4 offset = def.GetInputPortOffset(i, num_inputs) + PORT_ZBIAS;
            v4 radius = v4(def.InputPortRadius, def.InputPortRadius, 0.0f, 0.0f);

            SystemFunctions::DrawRectangle(wpos * (offset - radius), wpos * (offset + radius), v4(1.0f), true, true, true);
        }

        for (int i = 0; i < def.NumOutputs; ++i)
        {
            v4 offset = def.GetOutputPortOffset(i, def.NumOutputs) + PORT_ZBIAS;
            v4 radius = v4(def.OutputPortRadius, def.OutputPortRadius, 0.0f, 0.0f);

            SystemFunctions::DrawRectangle(wpos * (offset - radius), wpos * (offset + radius), v4(1.0f), true, false, true);
        }
    }
}

const CSwitchDefinition& GetSwitchDefinition(const CThing* thing)
{
    if (thing == NULL) return gDefaultSwitchDefinition;
    PSwitch* part_switch = thing->GetPSwitch();
    if (part_switch == NULL) return gDefaultSwitchDefinition;
    return part_switch->GetSwitchDefinition();
}

int GetNumInputs(const CThing* thing)
{
    if (thing == NULL) return 0;
    
    PSwitch* part_switch = thing->GetPSwitch();
    if (part_switch == NULL)
        return thing->GetPScript() != NULL ? 1 : 0;
    
    return part_switch->NumInputs;
}

MH_DefineFunc(PSwitch_RaycastConnector, 0x0004d764, TOC0, bool, PSwitch*, v4, v4, float&, CThing*&);
bool PSwitch::RaycastConnector(v4 start, v4 dir, float& t, CThing*& hit)
{
    return PSwitch_RaycastConnector(this, start, dir, t, hit);
}

const CSwitchDefinition& PSwitch::GetSwitchDefinition() const
{
    if (0 <= Type && Type < NUM_SWITCH_TYPES)
        return gSwitchDefinitions[Type];
    return gDefaultSwitchDefinition;
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

CSwitchOutput* CThing::GetInput(int port)
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
    new (&PortData) CVector<SPortData>();
    new (&ManualActivation) CSwitchSignal();
    Behaviour = 0;
    UpdateFrame = 0;
    OutputType = 0;
}

void PSwitch::DestroyExtraData()
{
    for (int i = 0; i < Outputs.size(); ++i)
        delete Outputs[i];
    
    Outputs.~CVector();
    PortData.~CVector();
}

void PSwitch::ClearLegacyData()
{
    TargetList.clear();
    PortData.clear();
    ConnectorGrabbed.clear();
    ConnectorPoints.clear();
    ConnectorPos.clear();
}

void PSwitch::GenerateLegacyData()
{
    ConnectorGrabbed.clear();
    ConnectorPos.clear();

    // If we're saving a switch, copy all thing references into
    // the target list, so references get cached and for compatibility
    // with retail.
    for (int i = 0; i < Outputs.size(); ++i)
    {
        CSwitchOutput& output = *Outputs[i];
        for (int j = 0; j < output.TargetList.size(); ++j)
        {
            CSwitchTarget& target = output.TargetList[j];
            const CSwitchDefinition& def = GetSwitchDefinition();

            ConnectorGrabbed.push_back(false);
            ConnectorPos.push_back(def.GetOutputPortOffset(j, def.NumOutputs));

            TargetList.push_back(target.Thing);
        }
    }

    // Generate all port mappings since we can't directly write them to the binary
    // without using serialization extensions.
    PortData.clear();
    for (int i = 0; i < Outputs.size(); ++i)
    {
        CSwitchOutput& output = *Outputs[i];
        for (int j = 0; j < output.TargetList.size(); ++j)
            PortData.push_back(SPortData(i, output.TargetList[j].Port));
    }
}

bool PSwitch::HasCustomData()
{
    // If anything is connected from an output
    // other than the first, then we'll have to
    // serialize extra data.
    for (int i = 1; i < Outputs.size(); ++i)
    {
        if (Outputs[i]->TargetList.size() != 0)
            return true;
    }

    // If we're targeting anything other than the first
    // port of another switch, then we need to serialize extra data.
    for (int i = 0; i < Outputs.size(); ++i)
    for (int j = 0; j < Outputs[i]->TargetList.size(); ++j)
    {
        if (Outputs[i]->TargetList[j].Port != 0)
            return true;
    }

    return false;
}

void PSwitch::OnPartLoaded()
{
    // Convert target list to output list, since we
    // don't serialize it directly to maintain
    // compatibility with retail levels.
    const CSwitchDefinition& def = GetSwitchDefinition();
    Outputs.try_resize(def.NumOutputs);
    for (int i = 0; i < def.NumOutputs; ++i)
    {
        CSwitchOutput* output = new CSwitchOutput();
        output->Owner = this;
        output->Port = i;

        Outputs[i] = output;
    }

    // If we didn't get any port data from the Alear extension data,
    // then it means everything is fine just targeting the first port
    // from the first output.
    if (PortData.empty())
    {
        CSwitchOutput* output = Outputs.front();
        output->TargetList.try_resize(TargetList.size());
        for (int i = 0; i < TargetList.size(); ++i)
            output->TargetList[i].Thing = TargetList[i];
    }
    // If we have port data though, it gives us the mappings for each port.
    else
    {
        for (int i = 0; i < TargetList.size(); ++i)
        {
            SPortData port_data = PortData[i];
            DebugLog("port remap from %d to %d\n", port_data.From, port_data.To);
            Outputs[port_data.From]->TargetList.push_back(CSwitchTarget(TargetList[i], port_data.To));
        }
    }

    ClearLegacyData();
}

bool CPoppetEditState::SetSwitchConnector(CThing* switch_thing, CThing* connector_thing)
{
    // switch_thing is RefThing
    // connector_thing is HitThing

    CPoppet* poppet = GetParent();

    if (switch_thing != NULL && connector_thing != NULL)
    {
        PSwitch* part_switch = switch_thing->GetPSwitch();

        SwitchConnectorRef = switch_thing;
        SwitchConnector = connector_thing;
        SwitchConnectorPort = poppet->Raycast.HitPort;
        SwitchConnectorRefPort = poppet->Raycast.RefPort;

        DebugLog("grabbing connector @ port %d\n", SwitchConnectorPort);

        return true;
    }

    SwitchConnector = NULL;
    SwitchConnectorRef = NULL;
    SwitchConnectorPort = -1;
    SwitchConnectorRefPort = -1;
    
    return false;
}

void CPoppetEditState::ClearSwitchConnector()
{
    SwitchConnector = NULL;
    SwitchConnectorRef = NULL;
    SetCursorHoverObject(NULL, -1);
}

CSwitchSignal PSwitch::GetActivationFromInput(int port)
{
    CSwitchSignal activation;

    CSwitchOutput* input = GetThing()->GetInput(port);
    if (input != NULL)
        activation = input->Activation;
    
    return activation;
}

CSwitchSignal PSwitch::GetNewActivation(int port)
{
    CSwitchSignal activation;

    float& analogue = activation.Analogue;
    int& ternary = activation.Ternary;
    int& player = activation.Player;

    switch (Type)
    {
        case SWITCH_TYPE_ALWAYS_ON:
        {
            activation = ManualActivation;
            break;
        }
        case SWITCH_TYPE_NOT:
        {
            activation = GetActivationFromInput(0);
            break;
        }
        case SWITCH_TYPE_OR:
        {
            CSwitchSignal a = GetActivationFromInput(0);
            CSwitchSignal b = GetActivationFromInput(1);

            analogue = abs(a.Analogue) < abs(b.Analogue) ? b.Analogue : a.Analogue;
            ternary = abs(b.Ternary) < abs(a.Ternary) || a.Ternary == -1 ? b.Ternary : a.Ternary;

            break;
        }
        case SWITCH_TYPE_AND:
        {
            // RandomOffTimeMin == 0 = min input
            //                  == 1 = multiply
            // 


            CSwitchSignal a = GetActivationFromInput(0);
            CSwitchSignal b = GetActivationFromInput(1);

            analogue = abs(a.Analogue) - abs(b.Analogue) < 0.0f ? abs(a.Analogue) : abs(b.Analogue);
            analogue *= ((a.Ternary == 0 ? 1 : a.Ternary) * (b.Ternary == 0 ? 1 : b.Ternary));
            ternary = abs(b.Ternary) < abs(a.Ternary) || a.Ternary == -1 ? b.Ternary : a.Ternary;

            break;
        }
    }

    if (Inverted)
    {
        switch (Type)
        {
            default:
            {
                analogue = analogue < 0.0f ? 1.0f + analogue : 1.0f - analogue;
                ternary = !ternary;
                break;
            }
        }
    }

    return activation;
}

void PSwitch::Update()
{
    for (CSwitchOutput** it = Outputs.begin(); it != Outputs.end(); ++it)
        (*it)->Activation = GetNewActivation((*it)->Port);


    PRenderMesh* part_render_mesh = GetThing()->GetPRenderMesh();
    if (Outputs.size() > 0 && part_render_mesh != NULL)
        part_render_mesh->EditorColour = v4(abs(Outputs[0]->Activation.Analogue));
}

void CPoppetEditState::UpdateSwitchConnector()
{
    CPoppet* poppet = GetParent();

    PSwitch* part_switch = SwitchConnectorRef->GetPSwitch();
    if (part_switch == NULL || SwitchConnector == NULL)
    {
        poppet->PopMode();
        return;
    }

    CThing* hit_thing = poppet->Raycast.HitThing;
    u8 hit_port = poppet->Raycast.HitPort;

    bool valid = hit_thing != NULL;

    bool is_loose = SwitchConnectorPort == 255;

    CInput* input = GetPlayer()->GetPYellowHead()->GetInput();
    if (input->IsJustClicked(BUTTON_CONFIG_POPPET_ACTION, L"BP_PLACE_SWITCH_HERE"))
    {
        if (valid)
        {
            poppet->Backup();

            if (!is_loose)
            {
                part_switch->Outputs[SwitchConnectorRefPort]->RemoveTarget(SwitchConnector, SwitchConnectorPort);
                part_switch->Outputs[SwitchConnectorRefPort]->AddTarget(hit_thing, hit_port);

                ClearSwitchConnector();
                poppet->PopMode();
            }
            else part_switch->Outputs[SwitchConnectorRefPort]->AddTarget(hit_thing, hit_port); 

            PlayPoppetEditSound("poppet/connect_switch", -10000.0f, -10000.0f);
        }
        else PlayPoppetEditSound("poppet/error_3d", -10000.0f, -10000.0f);
    }

    if (input->IsJustClicked(BUTTON_CONFIG_POPPET_CANCEL, (const wchar_t*)NULL))
    {
        PlayPoppetEditSound("poppet/cancelobject", -10000.0f, -10000.0f);
        ClearSwitchConnector();
        poppet->PopMode();
    }

    if (!is_loose && input->IsJustClicked(BUTTON_CONFIG_POPPET_DELETE, L"BP_DELETE_SWITCH"))
    {
        PlayPoppetEditSound("poppet/deleteobject", -10000.0f, -10000.0f);
        poppet->Backup();

        part_switch->Outputs[SwitchConnectorRefPort]->RemoveTarget(SwitchConnector, SwitchConnectorPort);

        ClearSwitchConnector();
        poppet->PopMode();

    }
}

MH_DefineFunc(RenderRope, 0x001ec5f0, TOC0, void, 
        RGfxMaterial* mat, 
        v2 const* points, 
        v2 const* oldpoints, 
        unsigned int numpoints,
        unsigned int stride_in_bytes,
        float width,
        float length,
        bool fixed_length,
        bool draw_cap_anyway,
        m44 const& trans,
        m44 const& old_trans,
        bool dissolving,
        v4 tint    
);

typedef CRawVector<const CThing*> UniqueThingVec;
extern UniqueThingVec gUniqueThingsOnCamera;
CRawVector<PRenderMesh*> gPortMeshticles;
class CMeshticleState {
public:
    m44 Spawn;
    v4 AABBMin;
    v4 AABBMax;
    s32 Life;
    float TargetScale;
    float PopY;
    u32 AirBubbleIndex;
};

class SHeldWire {
public:
    inline SHeldWire() : Source(NULL), Target(NULL), From(-1), To(-1) {}
    inline SHeldWire(CPoppet* poppet) : 
        Source(poppet->Edit.SwitchConnectorRef),
        Target(poppet->Edit.SwitchConnector),
        From(poppet->Edit.SwitchConnectorRefPort),
        To(poppet->Edit.SwitchConnectorPort)
        {}
public:
    CThing* Source;
    CThing* Target;
    int From;
    int To;
};

void CustomRenderWires()
{
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;

    // 0xcc7c - error wire
    // 0xadeb - default wire

    CP<RGfxMaterial> mat = LoadResourceByKey<RGfxMaterial>(0xadeb, 0, STREAM_PRIORITY_DEFAULT);
    mat->BlockUntilLoaded();

    v2 points[8];
    v4* points_v4 = (v4*)points;
    v4 tint(1.0f);
    m44 trans = m44::identity();

    cellGcmSetDepthMask(gCellGcmCurrentContext, CELL_GCM_TRUE);

    // Render all wires that any player is holding

    CVector<SHeldWire> held_wires(world->ListPYellowHead.size());
    for (PYellowHead** it = world->ListPYellowHead.begin(); it != world->ListPYellowHead.end(); ++it)
    {
        PYellowHead* yellowhead = *it;
        if (yellowhead == NULL) continue;

        CPoppet* poppet = yellowhead->Poppet;
        if (poppet == NULL || poppet->GetSubMode() != SUBMODE_SWITCH_CONNECTOR) continue;

        CThing* switch_thing = poppet->Edit.SwitchConnectorRef;
        int switch_port = poppet->Edit.SwitchConnectorRefPort;

        if (switch_thing == NULL) continue;

        held_wires.push_back(SHeldWire(poppet));

        const CSwitchDefinition& def = switch_thing->GetPSwitch()->GetSwitchDefinition();
        v4 port_offset = (switch_thing->GetPPos()->Game.WorldPosition * def.GetOutputPortOffset(switch_port, def.NumOutputs)) + PORT_ZBIAS;


        points_v4[0] = port_offset;
        points_v4[1] = poppet->Raycast.GetHitPoint();

        float dist = Vectormath::Aos::length(points_v4[1] - points_v4[0]);

        RenderRope(
            mat,
            points,
            points,
            2,
            sizeof(v2),
            25.0f,
            dist,
            false,
            true,
            trans,
            trans,
            false,
            tint
        );
    }

    // Render output wires
    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* part_switch = *it;
        if (part_switch == NULL) continue;
        CThing* thing = part_switch->GetThing();
        v4 pos = thing->GetPPos()->GetBestGameplayPosv4();

        const CSwitchDefinition& def = part_switch->GetSwitchDefinition();
        for (int i = 0; i < part_switch->Outputs.size(); ++i)
        {
            CSwitchOutput& output = *part_switch->Outputs[i];

            v4 start_point = (thing->GetPPos()->Game.WorldPosition * def.GetOutputPortOffset(i, def.NumOutputs)) + PORT_ZBIAS;

            for (int j = 0; j < output.TargetList.size(); ++j)
            {
                bool is_held_wire = false;
                CSwitchTarget& target = output.TargetList[j];

                for (SHeldWire* held_wire = held_wires.begin(); held_wire != held_wires.end(); ++held_wire)
                {
                    if (held_wire->Source == thing && held_wire->From == i && held_wire->To == target.Port && held_wire->Target == target.Thing)
                    {
                        is_held_wire = true;
                        break;
                    }
                }


                if (is_held_wire) continue;


                const CSwitchDefinition& target_def = GetSwitchDefinition(target.Thing);
                const int num_inputs = target.Thing->GetNumInputs();

                v4 end_point = (target.Thing->GetPPos()->Game.WorldPosition * def.GetInputPortOffset(target.Port, num_inputs)) + PORT_ZBIAS;
                
                

                bool below = start_point.getY() > end_point.getY();
                bool left = start_point.getX() > end_point.getX();

                if (target.Thing == thing)
                {
                    below = false;
                    left = true;
                }

                int num_points = Hermite(points_v4, 7, start_point, end_point,

                    500.0f,
                    below ? -500.0f : (left ? 500.0f : 0.0f),
                    500.0f,
                    0.0f

                    // 1000.0f,
                    // below ? -1000.0f : (left ? 0.0f : 1000.0f),
                    // 1000.0f,
                    // 0.0f
                );

                points_v4[num_points++] = end_point;

                float dist = 0.0f;
                for (int i = 1; i < num_points; ++i)
                    dist += Vectormath::Aos::length(points_v4[i] - points_v4[i - 1]);

                RenderRope(
                    mat,
                    points,
                    points,
                    num_points,
                    sizeof(v2),
                    25.0f,
                    dist,
                    false,
                    true,
                    trans,
                    trans,
                    false,
                    tint
                );

            }
        }
    }


    cellGcmSetDepthMask(gCellGcmCurrentContext, CELL_GCM_FALSE);
}

void CPoppet::RaycastAgainstSwitchConnector(v4 ray_start, v4 ray_dir, CRaycastResults& results)
{
    if (Edit.SwitchConnectorRef == NULL) return;
    PSwitch* part_switch = Edit.SwitchConnectorRef->GetPSwitch();
    if (part_switch == NULL) return;

    results.RefPort = -1;
    results.HitPort = -1;

    PWorld* world = gGame->GetWorld();

    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* sw = (*it);
        CThing* thing = sw->GetThing();

        float t;

        m44 wpos = thing->GetPPos()->Game.WorldPosition;
        const CSwitchDefinition& def = sw->GetSwitchDefinition();

        int num_inputs = def.NumInputs;
        if (sw->Type == SWITCH_TYPE_AND || sw->Type == SWITCH_TYPE_OR || sw->Type == SWITCH_TYPE_XOR || sw->Type == SWITCH_TYPE_SELECTOR)
            num_inputs = sw->NumInputs;

        for (int i = 0; i < num_inputs; ++i)
        {
            v4 offset = (wpos * def.GetInputPortOffset(i, num_inputs)) + PORT_ZBIAS;
            if (!NPoppetUtils::RaySphereIntersect(offset, def.InputPortRadius, ray_start, ray_dir, t)) continue;
            if (t >= 1.0e+20f) continue;


            results.BaryU = 0.0f;
            results.BaryV = 0.0f;
            results.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
            results.TriIndex = 0;
            results.OnCostumePiece = -1;
            results.DecorationIdx = -1;

            results.HitPort = i;
            results.HitThing = thing;

            m_bestT = t;
        }
    }
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

        // Test against our own output ports first
        m44 wpos = thing->GetPPos()->Game.WorldPosition;
        const CSwitchDefinition& def = sw->GetSwitchDefinition();
        for (int i = 0; i < def.NumOutputs; ++i)
        {
            v4 offset = (wpos * def.GetOutputPortOffset(i, def.NumOutputs)) + PORT_ZBIAS;
            if (!NPoppetUtils::RaySphereIntersect(offset, def.OutputPortRadius, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
            if (t >= 1.0e+20f) continue;

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

            raycast.HitPort = -1;
            raycast.RefPort = i;
        }

        // Test against each of our targets to see if we're trying to select a connected wire.
        for (int ref_port = 0; ref_port < sw->Outputs.size(); ++ref_port)
        {
            CSwitchOutput& output = *sw->Outputs[ref_port];
            for (CSwitchTarget* target = output.TargetList.begin(); target != output.TargetList.end(); ++target)
            {
                CThing* connector_thing = target->Thing;
                if (connector_thing == NULL) continue;

                const CSwitchDefinition& connector_def = GetSwitchDefinition(connector_thing);

                v4 offset = (connector_thing->GetPPos()->Game.WorldPosition * connector_def.GetInputPortOffset(target->Port, GetNumInputs(connector_thing))) + PORT_ZBIAS;
                if (!NPoppetUtils::RaySphereIntersect(offset, def.InputPortRadius, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
                DebugLog("hit an input!!!\n");
                if (t >= 1.0e+20f) continue;

                raycast.SwitchConnector = true;
                raycast.HitThing = connector_thing;
                raycast.RefThing = thing;

                poppet->m_bestTFromPSwitches = t;
                poppet->m_havePSwitchHit = true;
    
                raycast.BaryU = 0.0f;
                raycast.BaryV = 0.0f;
                raycast.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
                raycast.TriIndex = 0;
                raycast.OnCostumePiece = -1;
                raycast.DecorationIdx = -1;
    
                raycast.RefPort = ref_port;
                raycast.HitPort = target->Port;
            }
        }
    }
}

// lbp2 style blah blah
// if you hover over output
    // RefThing is set to thing
    // RefPort is set to port number
    // HitThing is NULL
    // HitPort is -1
// if you hover over an input
    // HitThing is attached wire thing
    // HitPort is attached wire port
    // RefThing is the thing the wire comes from
    // RefPort is the port the wire comes from

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

ReflectReturn GatherSwitchVariables(CGatherVariables& r, PSwitch& d)
{
    ReflectReturn ret;
    ADD(Outputs);
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
extern "C" uintptr_t _render_wire_hook;
extern "C" uintptr_t _switch_gather_variables_hook;

void OnPartSwitchDestructor(PSwitch* sw)
{
    sw->DestroyExtraData();
}


namespace LogicSystemNativeFunctions
{
    int GetSwitchType(CThing* thing)
    {
        if (thing == NULL) return SWITCH_TYPE_INVALID;
        PSwitch* part_switch = thing->GetPSwitch();
        return part_switch != NULL ? part_switch->Type : SWITCH_TYPE_INVALID;
    }

    int GetNumOutputs(CThing* thing)
    {
        if (thing == NULL) return 0;
        PSwitch* part_switch = thing->GetPSwitch();
        return part_switch != NULL ? part_switch->Outputs.size() : 0;
    }

    int GetTweakTitle(CThing* thing)
    {
        const int E_KEY_SWITCH = 2009090952; 
        if (thing == NULL) return E_KEY_SWITCH;
        PSwitch* part_switch = thing->GetPSwitch();
        if (part_switch == NULL) return E_KEY_SWITCH;

        const char* prefix = NULL;
        bool inverted = part_switch->Inverted;

        // stupid hack
        #ifdef NOP
            #undef NOP
        #endif
        #define GET_PREFIX(name) case SWITCH_TYPE_##name: prefix = "SWITCH_TYPE_" #name; break;
        #define GET_PREFIX_WITH_INVERSE(name, inverse) case SWITCH_TYPE_##name: prefix = inverted ? "SWITCH_TYPE_" #inverse : "SWITCH_TYPE_" #name; break; 
        #define GET_PREFIX_WITH_INVERSE_FLIPPED(name, inverse) case SWITCH_TYPE_##name: prefix = inverted ? "SWITCH_TYPE_" #name : "SWITCH_TYPE_" #inverse; break; 


        switch (part_switch->Type)
        {
            GET_PREFIX_WITH_INVERSE(AND, NAND)
            GET_PREFIX_WITH_INVERSE(OR, NOR)
            GET_PREFIX_WITH_INVERSE_FLIPPED(NOT, NOP)
            GET_PREFIX(ALWAYS_ON)
            GET_PREFIX(TOGGLE)
            GET_PREFIX(TIMER)
            GET_PREFIX(DIRECTION)
            GET_PREFIX(SIGN_SPLIT)
        }

        #undef GET_PREFIX
        #undef GET_PREFIX_WITH_INVERSE

        if (prefix == NULL) return E_KEY_SWITCH;

        return MakeLamsKeyID(prefix, "_NAME");
    }

    void Register()
    {
        RegisterNativeFunction("SwitchBase", "GetSwitchType__", false, NVirtualMachine::CNativeFunction1<int, CThing*>::Call<GetSwitchType>);
        RegisterNativeFunction("SwitchBase", "GetNumOutputs__", false, NVirtualMachine::CNativeFunction1<int, CThing*>::Call<GetNumOutputs>);
        RegisterNativeFunction("SwitchBase", "GetTweakTitle__", false, NVirtualMachine::CNativeFunction1<int, CThing*>::Call<GetTweakTitle>);
    }
}


void InitLogicSystemHooks()
{
    LogicSystemNativeFunctions::Register();
    InitializeSwitchDefinitions();

    MH_InitHook((void*)0x00353370, (void*)&CustomRaycastAgainstSwitches);
    MH_InitHook((void*)0x007716e8, (void*)&GatherPoppetRaycastVariables);

    MH_PokeMemberHook(0x00365a0c, CPoppetEditState::SetSwitchConnector);
    MH_PokeMemberHook(0x003759dc, CPoppetEditState::UpdateSwitchConnector);
    MH_PokeMemberHook(0x0034aa24, CPoppet::RaycastAgainstSwitchConnector);
    MH_PokeMemberHook(0x00078ed8, PSwitch::Update);
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
    MH_PokeBranch(0x001df8f4, &_render_wire_hook);

    MH_PokeBranch(0x00740884, &_switch_gather_variables_hook);
}