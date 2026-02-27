#include "Hermite.h"
#include "AlearHooks.h"

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
#include <DebugLog.h>

#include <cell/gcm.h>
#include <gfxcore.h>
#include <mmalex.h>

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

#include <SceneGraph.h>

const v4 PORT_ZBIAS = v4(0.0f, 0.0f, 10.0f, 0.0f);
const float PORT_RADIUS = 20.0f;

namespace NPoppetUtils
{
    MH_DefineFunc(RaySphereIntersect, 0x0038eedc, TOC1, bool, v4 center, float radius, v4 ray_pos, v4 ray_dir, float& intersect_dist);
}

MH_DefineFunc(HitTestRayVsAABB, 0x002272c4, TOC0, bool, v4 boxmin, v4 boxmax, v4 origin, v4 dir, float* tout, v4* hitout);

namespace SystemFunctions
{
    MH_DefineFunc(DrawRectangle, 0x0046369c, TOC1, void, v4 min, v4 max, v4 col, bool world_space, bool z_test, bool fill);
}

m44 GetUniformMatrix(const m44& transform)
{
    m33 mat3 = transform.getUpper3x3();
    mat3[0] /= Vectormath::Aos::length(mat3[0]);
    mat3[0] *= Vectormath::Aos::length(mat3[1]);
    m44 mat4 = m44::identity();
    mat4.setUpper3x3(mat3);
    mat4.setCol3(transform.getCol3());
    
    return mat4;
}

float Normalize(v3& col)
{
    float d = Vectormath::Aos::dot(col, col);
    if (d > 1.0e-35f)
    {
        d = mmalex::sqrtf(d);
        col *= 1.0f / d;
    }
    else
    {
        col = v3(0.0f);
        d = 0.0f;
    }

    return d;
}

void Decompose(v4& translation, m44& rotation, v3& scale, const m44& transform)
{
    m33 mat3 = transform.getUpper3x3();
    
    scale.setX(Normalize(mat3[0]));
    scale.setY(Normalize(mat3[1]));
    scale.setZ(Normalize(mat3[2]));

    if (Vectormath::Aos::determinant(rotation) < 0.0f)
    {
        mat3 *= -1.0f;
        scale *= -1.0f;
    }

    rotation = m44::identity();
    rotation.setUpper3x3(mat3);

    translation = transform.getCol3();
}

void Decompose(v4& translation, q4& rotation, v3& scale, const m44& transform)
{
    m33 mat3 = transform.getUpper3x3();

    scale.setX(Normalize(mat3[0]));
    scale.setY(Normalize(mat3[1]));
    scale.setZ(Normalize(mat3[2]));
    if (Vectormath::Aos::determinant(mat3) < 0.0f)
    {
        mat3 *= -1.0f;
        scale *= -1.0f;
    }

    rotation = q4(mat3);
    translation = transform.getCol3();
}

int GetNumOutputs(PSwitch* sw)
{
    switch (sw->Type)
    {
        case SWITCH_TYPE_SIGN_SPLIT:
            return 2;
        case SWITCH_TYPE_SELECTOR:
            return sw->InputCount;
    }

    return 1;
}

int GetTopInputs(const CThing* thing)
{
    return 0;
}

int GetBottomInputs(const CThing* thing)
{
    switch (thing->ObjectType)
    {
        case OBJECT_SWITCH_SELECTOR: return 1;
    }

    return 0;
}

int GetNumInputs(const CThing* thing)
{
    switch (thing->ObjectType)
    {
        case OBJECT_TRIGGER_DELETE:
        case OBJECT_CREDITS_TEXT:
        case OBJECT_CREDITS_LIGHTS_ON:
        case OBJECT_CREDITS_LIGHTS_OFF:
        case OBJECT_MAGNETIC_KEY:
        case OBJECT_LIGHT:
        case OBJECT_EXPLOSIVE_IMPACT:
        case OBJECT_EXPLOSIVE_TRIGGERED:
        case OBJECT_DISSOLVABLE:
        case OBJECT_EXPLODING:
        case OBJECT_MESH_GENERATED_TWEAKABLE:
        case OBJECT_MESH_GENERATED_HOLOGRAM:
        case OBJECT_MESH_GENERATED_STICKER_PANEL:
        case OBJECT_MESH_GENERATED_OUTLINE:
        case OBJECT_MESH_GENERATED_SILHOUETTE:
        case OBJECT_MESH_TWEAKABLE:
        case OBJECT_JOINT_MOTOR_BOLT:
        case OBJECT_JOINT_WOBBLE_BOLT:
        case OBJECT_JOINT_FLIPPER_BOLT:
        case OBJECT_JOINT_CHAIN:
        case OBJECT_JOINT_PISTON:
        case OBJECT_CAMERA_GAME:
        case OBJECT_CAMERA_MOVIE:
        case OBJECT_SWITCH_TOGGLE:
        case OBJECT_SWITCH_RANDOM:
        case OBJECT_SWITCH_NOT:
        case OBJECT_SWITCH_NOP:
        case OBJECT_SWITCH_SIGN_SPLIT:
        case OBJECT_SWITCH_ANIMATIC:
        case OBJECT_SWITCH_CIRCUIT_NODE:
        case OBJECT_LETHALISER:
        case OBJECT_DESTROYER:
        case OBJECT_SACKBOT_BEHAVIOUR:
        case OBJECT_SOUND:
        case OBJECT_MUSIC_NORMAL:
        case OBJECT_MUSIC_INTERACTIVE:
        case OBJECT_EMITTER:
        case OBJECT_PHYSICS_TWEAK:
        case OBJECT_MATERIAL_TWEAK:
        case OBJECT_SMOKE_MACHINE:
        case OBJECT_THRUSTER:
        case OBJECT_BUBBLE_MACHINE:
        case OBJECT_GLOBAL_LIGHT_TWEAK:
        case OBJECT_GLOBAL_WATER_TWEAK:
        case OBJECT_GLOBAL_AUDIO_TWEAK:
        case OBJECT_GLOBAL_GRAVITY_TWEAK:
        case OBJECT_SCORE_GIVER:
        case OBJECT_GAME_ENDER:
        case OBJECT_BOUNCE_PAD:
        case OBJECT_MOVER:
        case OBJECT_FOLLOWER:
        case OBJECT_IN_OUT_MOVER:
        case OBJECT_GYROSCOPE:
        case OBJECT_LOOK_AT_ROTATOR:
        case OBJECT_ROCKET_ROTATOR:
        case OBJECT_ROTATOR:
        case OBJECT_CUSTOM_NOTE:
        case OBJECT_CREATURE_BRAIN_BASE:
        case OBJECT_MAGIC_MOUTH:
        case OBJECT_MAGIC_EYE:
        case OBJECT_SPAWNPOINT_ENTRANCE:
        case OBJECT_SPAWNPOINT_SINGLE_LIFE:
        case OBJECT_SPAWNPOINT_DOUBLE_LIFE:
        case OBJECT_SPAWNPOINT_INFINITE_LIFE:
        case OBJECT_SCOREBOARD:
        case OBJECT_LEVEL_LINK:
        case OBJECT_CLOSE_LEVEL:
        case OBJECT_SCORE_BUBBLE:
        case OBJECT_PRIZE_BUBBLE:
        case OBJECT_KEY:
        case OBJECT_RACE_START:
        case OBJECT_RACE_END:
        case OBJECT_BULLET_WATER:
        case OBJECT_TUTORIAL_MONITOR:
        case OBJECT_MOTION_RECORDER:
        case OBJECT_PIN_GIVER:
        case OBJECT_ATTRACTOR:
        case OBJECT_WORM_HOLE:
        case OBJECT_MESH_GENERATED_TOUCH:
        case OBJECT_STATICCLING_TWEAK:
        case OBJECT_SPINNING_OBJECT:
        case OBJECT_MESH_GENERATED_DCCOMICS_SPEED:
        case OBJECT_MESH_GENERATED_DCCOMICS_LANTERNPOWER:
        case OBJECT_MESH_GENERATED_DCCOMICS_FIREFLAKES:
        case OBJECT_TARGET_SCRIPTED_LEGACY:
            return 1;

        case OBJECT_SWITCH_COUNTDOWN:
        case OBJECT_SWITCH_TIMER:
        case OBJECT_SWITCH_DIRECTION:
        case OBJECT_ADVANCED_ROTATOR:
            return 2;

        case OBJECT_ADVANCED_MOVER:
        case OBJECT_JOYSTICK_ROTATOR:
            return 3;

        case OBJECT_SWITCH_AND:
        case OBJECT_SWITCH_OR:
        case OBJECT_SWITCH_XOR:
            return thing->GetPSwitch()->InputCount;
        
        case OBJECT_SWITCH_SELECTOR:
            return thing->GetPSwitch()->Outputs.size() + 1;
    }

    return 0;
}

CBone* GetRootBone(const CThing* thing)
{
    if (thing == NULL) return NULL;
    
    const CMesh* mesh;
    if (thing->GetPGeneratedMesh() != NULL) mesh = thing->GetPGeneratedMesh()->SharedMesh;
    else
    {
        PRenderMesh* part_render_mesh;
        if (thing == NULL || (part_render_mesh = thing->GetPRenderMesh()) == NULL) return NULL; 
        if (!part_render_mesh->Mesh || !part_render_mesh->Mesh->IsLoaded()) return NULL;

        mesh = &part_render_mesh->Mesh->mesh;
    }

    if (mesh == NULL || mesh->Bones.size() == 0) return NULL;
    return mesh->Bones.begin();
}

bool RaycastPort(CThing* thing, int port, int num_ports, bool switch_connector, bool output, v4 ray_pos, v4 ray_dir, float& intersect_dist)
{
    if (thing == NULL) return false;

    bool centered = false;

    const CMesh* mesh;
    if (thing->GetPGeneratedMesh() != NULL) 
    {
        mesh = thing->GetPGeneratedMesh()->SharedMesh;
        centered = num_ports == 1 && !output;
    }
    else
    {
        PRenderMesh* part_render_mesh;
        if (thing == NULL || (part_render_mesh = thing->GetPRenderMesh()) == NULL) return false;
        if (!part_render_mesh->Mesh || !part_render_mesh->Mesh->IsLoaded()) return false;

        mesh = &part_render_mesh->Mesh->mesh;
    }

    // There technically shouldn't ever normally be a case where
    // the model doesn't have any bones due to how LBP works in general,
    // but we'll still check it anyways, I guess.
    if (mesh == NULL || mesh->Bones.size() == 0) return false;

    CBone& bone = mesh->Bones.front();
    const m44& wpos = thing->GetPPos()->GetWorldPosition();

    v4 center = bone.SkinPoseMatrix.getCol3();

    float dist = abs(bone.BoundBoxMax.getY() - bone.BoundBoxMin.getY());
    float inc = dist / (float)num_ports;

    float top = bone.BoundBoxMax.getY() - (inc * (float)(port + 0));
    float bottom = bone.BoundBoxMax.getY() - (inc * (float)(port + 1));

    // If we're raycasting against this port while trying to connect a switch,
    // we'll split the mesh's bound box for a boxcast.
    if (!output && switch_connector)
    {
        v4 bl, tr;

        if (centered)
        {
            bl = bone.BoundBoxMin;
            tr = bone.BoundBoxMax;
        }
        else
        {
            bl = v4(bone.BoundBoxMin.getX() - PORT_RADIUS * 2.0f, bottom, center.getZ(), 1.0f);
            tr = v4(bone.BoundBoxMax.getX(), top, center.getZ(), 1.0f);
        }
        
        v4 hitout;
        float tout;
        bool hit =  HitTestRayVsAABB(wpos * bl, wpos * tr, ray_pos, ray_dir, &tout, NULL);

        if (!hit || intersect_dist < tout) return false;

        intersect_dist = tout;
        return true;
    }

    // Otherwise, we'll do a sphere cast against the port.

    if (centered)
        return NPoppetUtils::RaySphereIntersect(wpos * center, PORT_RADIUS, ray_pos, ray_dir, intersect_dist);

    float origin = output ? bone.BoundBoxMax.getX() + PORT_RADIUS : bone.BoundBoxMin.getX() - PORT_RADIUS;
    return NPoppetUtils::RaySphereIntersect(wpos * v4(origin, (top + bottom) / 2.0f, center.getZ(), 1.0f), PORT_RADIUS, ray_pos, ray_dir, intersect_dist);
}

void RenderSwitchDebug()
{
    return;
    
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;

    for (CThing** it = world->Things.begin(); it != world->Things.end(); ++it)
    {
        CThing* thing = *it;
        if (thing == NULL || thing->GetPPos() == NULL || thing->Stamping) continue;

        int num_inputs = GetNumInputs(thing);
        if (num_inputs == 0) continue;

        m44& wpos = thing->GetPPos()->Game.WorldPosition;

        CBone* bone = GetRootBone(thing);
        if (bone == NULL) continue;

        v4 center = bone->SkinPoseMatrix.getCol3() + PORT_ZBIAS;

        float dist = abs(bone->BoundBoxMax.getY() - bone->BoundBoxMin.getY());
        float inc = dist / (float)num_inputs;

        
        
        SystemFunctions::DrawRectangle(wpos * (bone->BoundBoxMin + PORT_ZBIAS), wpos * (bone->BoundBoxMax + PORT_ZBIAS), v4(1.0f, 0.0f, 0.0f, 1.0f), true, true, true);
        continue;
        


        for (int i = 0; i < num_inputs; ++i)
        {
            float top = bone->BoundBoxMax.getY() - (inc * (float)(i + 0));
            float bottom = bone->BoundBoxMax.getY() - (inc * (float)(i + 1));

            v4 bl(bone->BoundBoxMin.getX() - PORT_RADIUS * 2.0f, ((top + bottom) / 2.0f) - PORT_RADIUS, center.getZ(), 1.0f);
            v4 tr(bone->BoundBoxMin.getX(), ((top + bottom) / 2.0f) + PORT_RADIUS, center.getZ(), 1.0f);

            v4 color;
            switch (i % 3)
            {
                case 0: color = v4(1.0f, 0.0f, 0.0f, 1.0f); break;
                case 1: color = v4(0.0f, 1.0f, 0.0f, 1.0f); break;
                case 2: color = v4(0.0f, 0.0f, 1.0f, 1.0f); break;
            }

            SystemFunctions::DrawRectangle(wpos * bl, wpos * tr, color, true, true, true);
        }

        PSwitch* sw = thing->GetPSwitch();
        if (sw == NULL) continue;

        int num_outputs = sw->Outputs.size();
        inc = dist / (float)num_outputs;

        for (int i = 0; i < num_outputs; ++i)
        {
            float top = bone->BoundBoxMax.getY() - (inc * (float)(i + 0));
            float bottom = bone->BoundBoxMax.getY() - (inc * (float)(i + 1));

            v4 bl(bone->BoundBoxMax.getX(), ((top + bottom) / 2.0f) - PORT_RADIUS, center.getZ(), 1.0f);
            v4 tr(bone->BoundBoxMax.getX() + PORT_RADIUS * 2.0f, ((top + bottom) / 2.0f) + PORT_RADIUS, center.getZ(), 1.0f);

            SystemFunctions::DrawRectangle(wpos * bl, wpos * tr, v4(1.0f), true, true, true);
        }
    }
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

void PSwitch::ClearLegacyData()
{
    TargetList.clear();
    ConnectorGrabbed.clear();
    ConnectorPoints.clear();
    ConnectorPos.clear();
}

void PSwitch::GenerateLegacyData()
{
    ConnectorGrabbed.clear();
    ConnectorPos.clear();

    DeprecatedManualActivation = ManualActivation.Analogue;

    // If we're saving a switch, copy all thing references into
    // the target list, so references get cached and for compatibility
    // with retail.
    for (int i = 0; i < Outputs.size(); ++i)
    {
        CSwitchOutput& output = *Outputs[i];
        
        if (i == 0)
            Activation = output.Activation.Analogue;
        
        for (int j = 0; j < output.TargetList.size(); ++j)
        {
            CSwitchTarget& target = output.TargetList[j];

            ConnectorGrabbed.push_back(false);
            ConnectorPos.push_back(v4(0.0f));

            TargetList.push_back(target.Thing);
        }
    }
}

void PSwitch::OnPartLoaded()
{
    // If the outputs array isn't empty, it means it got
    // populated from the data extensions.
    if (Outputs.size() != 0)
    {
        ClearLegacyData();
        return;
    }

    // Otherwise if it is empty, means we're working with legacy data,
    // so construct our outputs and assign the target list to them.

    Outputs.try_resize(GetNumOutputs(this));
    for (int i = 0; i < Outputs.size(); ++i)
    {
        CSwitchOutput* output = new CSwitchOutput();
        output->Owner = this;
        output->Port = i;

        Outputs[i] = output;
    }

    CSwitchOutput* output = Outputs.front();
    
    output->Activation.Analogue = Activation;
    ManualActivation.Analogue = DeprecatedManualActivation;
    
    output->TargetList.try_reserve(TargetList.size());
    for (int i = 0; i < TargetList.size(); ++i)
    {
        if (TargetList[i] == NULL) continue;
        output->TargetList.push_back(CSwitchTarget(TargetList[i], 0));
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

        // dumb hack
        poppet->Raycast.HitThing = NULL;
        poppet->Raycast.HitPort = -1;

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

void PerformReactionStatic(CSwitchOutput* input, CThing* target, int port, bool one_shot)
{
    switch (target->ObjectType)
    {
        case OBJECT_SWITCH_SELECTOR:
        {
            if (!one_shot) return;

            PSwitch* sw = target->GetPSwitch();
            if (sw == NULL || port != 0) return;

            sw->ManualActivation.Player = input->Activation.Player;
            sw->SelectorState = (sw->SelectorState + input->Activation.Ternary) % sw->Outputs.size();

            break;
        }
        case OBJECT_EMITTER:
        {
            if (!one_shot) return;

            PEmitter* part_emitter = target->GetPEmitter();
            if (part_emitter == NULL) return;

            if (target->Behaviour != EMITTER_BEHAVIOR_ONE_SHOT) return;

            part_emitter->AttemptFire(false, NULL);

            break;
        }
    }
}

static CSwitchSignal EMPTY_ACTIVATION;
const CSwitchSignal& PSwitch::GetActivation(int port)
{
    if (port < Outputs.size())
        return Outputs[port]->Activation;
    return EMPTY_ACTIVATION;
}

CSwitchSignal PSwitch::GetActivationFromInput(int port)
{
    CSwitchSignal activation;

    CSwitchOutput* input = GetThing()->GetInput(port);
    if (input != NULL)
        activation = input->Activation;
    
    return activation;
}

MH_DefineFunc(GetNewActivationButton, 0x0006e7c8, TOC0, float, PSwitch*);
MH_DefineFunc(GetNewActivationProximity, 0x0006b45c, TOC0, float, PSwitch*);

extern void (*ForceSum)(const PShape* shape, unsigned int n, v2& sum, float& length_sum, float min_vel_length);

CSwitchSignal PSwitch::GetNewActivationButton(bool b)
{
    CSwitchSignal activation;
    if (!ReferenceThing) return activation;

    v2 sum; float length_sum;
    ForceSum(ReferenceThing->GetPShape(), 0, sum, length_sum, -FLT_MAX);

    int player = GetActivation(0).Player;
    float analogue = MAX(0.0, MIN(1.0, length_sum / 10000.0));
    int ternary = RemapTernary(analogue, 0, 0);
    if (b && ActivationHoldTime < 4 && ternary == 0)
    {
        const CSwitchSignal& old_activation = GetActivation(0);
        if (old_activation.Ternary == !Inverted)
        {
            analogue = old_activation.Analogue;
            player = old_activation.Player;
            ternary = old_activation.Ternary;

            if (Inverted)
            {
                analogue = analogue < 0.0 ? analogue + 1.0 : 1.0 - analogue;
                ternary = !ternary;
            }
        }

    }

    activation.Player = player;
    activation.Analogue = analogue;
    activation.Ternary = ternary;

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
        case SWITCH_TYPE_BUTTON:
        {
            activation = GetNewActivationButton(true);
            break;
        }
        case SWITCH_TYPE_PROXIMITY:
        {
            analogue = GetNewActivationProximity(this);
            ternary = analogue < 0.0f ? -1 : analogue > 0.0f ? 1 : 0; 
            player = E_PLAYER_NUMBER_NONE; 
            break;
        }
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
        case SWITCH_TYPE_SELECTOR:
        {
            if (SelectorState == port)
            {
                player = ManualActivation.Player;
                ternary = 1;
                analogue = 1.0f;
            }

            break;
        }
        case SWITCH_TYPE_SIGN_SPLIT:
        {
            CSwitchSignal input = GetActivationFromInput(0);
            player = input.Player;
            if (port == 0)
            {
                analogue = -input.Analogue;
                ternary = -input.Ternary;
                if (analogue <= 0.0f)
                {
                    ternary = ternary > 0 ? 1 : 0;
                    analogue = 0.0f;
                }
                else if (ternary < 0) ternary = 0;
            }
            else
            {
                analogue = input.Analogue > 0.0f ? input.Analogue : 0.0f;
                ternary = input.Ternary > 0 ? 1 : 0;
            }
            
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

int PSwitch::RemapTernary(float activation, int, int port)
{
    if ((Type == SWITCH_TYPE_CONTROL_PAD || Type ==  SWITCH_TYPE_MOVINATOR_PAD) && (port == 6 || port == 9))
        return 0.0f < activation ? 1 : 0;

    if (Type == SWITCH_TYPE_KEY || Type == SWITCH_TYPE_KEY_REMOTE || Type == SWITCH_TYPE_PROXIMITY)
        activation = activation * 3.75f;

    if (Type == SWITCH_TYPE_COUNTDOWN || Type == SWITCH_TYPE_SCORE || Type == SWITCH_TYPE_ANGLE || Type == SWITCH_TYPE_VELOCITY_LINEAR || Type == SWITCH_TYPE_VELOCITY_ANGULAR)
        return abs(activation) < 1.0f ? 0 : activation < 0.0 ? -1 : 1;
    
    float f = 0.0f;
    if (Type != SWITCH_TYPE_PAINT && Type != SWITCH_TYPE_PROJECTILE && Type != SWITCH_TYPE_TIMER)
        f = 0.25f;
    
    if (activation <= f && -f <= activation)
        return 0;
    if (1.0 - f <= activation)
        return 1;
    if (activation <= f - 1.0)
        return -1;

    if (!Inverted) return Outputs[port]->Activation.Ternary;

    switch (Type)
    {
        default: return !Outputs[port]->Activation.Ternary;
    }
}

void PSwitch::SetInputCount(int size)
{
    CThing* thing = GetThing();

    if (Type == SWITCH_TYPE_SELECTOR)
    {
        for (int i = Outputs.size() - 1; i >= size; --i)
            RemoveOutput(i);

        Outputs.try_resize(size);
        for (int i = 0; i < size; ++i)
        {
            if (Outputs[i] != NULL) continue;
            Outputs[i] = new CSwitchOutput();
            Outputs[i]->Port = i;
            Outputs[i]->Owner = this;
        }

    }

    for (int i = InputCount - 1; i >= size; --i)
    {
        CSwitchOutput* input = thing->GetInput(i);
        if (input != NULL)
            input->RemoveTarget(thing, i);
    }

    InputCount = size;
}

void PSwitch::RemoveOutput(int port)
{
    if (port < Outputs.size())
    {
        if (Outputs[port] != NULL) delete Outputs[port];
        Outputs.erase(Outputs.begin() + port);
        for (int i = 0; i < Outputs.size(); ++i)
            Outputs[i]->Port = i;
    }
}

void PSwitch::Update()
{
    if (Type == SWITCH_TYPE_SELECTOR)
    {
        v4 translation; v3 scale; m44 rotation;
        Decompose(translation, rotation, scale, GetThing()->GetPPos()->Game.WorldPosition);

        scale.setX(InputCount * scale.getY());

        m44 wpos = rotation * m44::scale(scale);
        wpos.setCol3(translation);
        GetThing()->GetPPos()->SetWorldPos(wpos, false, 0);
    }

    if (GetThing()->Stamping) return;
    
    if (Type == SWITCH_TYPE_SELECTOR && Outputs.size() - 1 >= 0)
    {
        for (int port = Outputs.size() - 1; port >= 0; --port)
        {
            CSwitchSignal activation = GetActivationFromInput(port + 1);
            if (activation.Ternary != 0)
            {
                SelectorState = port;
                ManualActivation.Player = activation.Player;
                break;
            }
        }
    }

    for (CSwitchOutput** it = Outputs.begin(); it != Outputs.end(); ++it)
    {
        CSwitchOutput* output = *it;

        CSwitchSignal old = output->Activation;
        output->Activation = GetNewActivation(output->Port);

        bool one_shot = (!old.Ternary && output->Activation.Ternary) || CrappyOldLBP1Switch;
        bool change = old.Ternary != output->Activation.Ternary || old.Player != output->Activation.Player;
        if (change || one_shot)
        {
            for (CSwitchTarget* target = output->TargetList.begin(); target != output->TargetList.end(); ++target)
                PerformReactionStatic(output, target->Thing, target->Port, one_shot);
        }
    }

    CrappyOldLBP1Switch = false;

    switch (Type)
    {
        case SWITCH_TYPE_BUTTON:
        {

            break;
        }
    }

    PRenderMesh* part_render_mesh = GetThing()->GetPRenderMesh();
    if (Outputs.size() > 0 && part_render_mesh != NULL)
    {
        float activation = abs(Outputs[0]->Activation.Analogue);
        if (Type == SWITCH_TYPE_SIGN_SPLIT)
        {
            activation = 0.0f;
            if (Outputs[0]->Activation.Ternary)
                activation = 1.0f / 3.0f;
            else if (Outputs[1]->Activation.Ternary)
                activation = 2.0f / 3.0f;
        }
        else if (Type == SWITCH_TYPE_SELECTOR)
            activation = InputCount / 10.0f;
        else if (Type == SWITCH_TYPE_ANGLE)
            activation = AngleRange / 180.0f;
        
        part_render_mesh->EditorColour = v4(activation);
    }
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

    if (!is_loose)
    {
        if (input->IsJustClicked(BUTTON_CONFIG_POPPET_DELETE, L"BP_DELETE_SWITCH"))
        {
            PlayPoppetEditSound("poppet/deleteobject", -10000.0f, -10000.0f);
            poppet->Backup();
    
            part_switch->Outputs[SwitchConnectorRefPort]->RemoveTarget(SwitchConnector, SwitchConnectorPort);
    
            ClearSwitchConnector();
            poppet->PopMode();
        }

        if (input->IsJustClicked(BUTTON_CONFIG_POPPET_COPY, (const wchar_t*)NULL))
        {
            PlayPoppetEditSound("poppet/createstamp", -10000.0f, -10000.0f);

            SwitchConnector = SwitchConnectorRef;
            SwitchConnectorPort = -1;
        }
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

bool GetPortPos(const CThing* thing, int port, bool output, v4& out)
{
    if (port == 255) return false;

    CBone* bone = GetRootBone(thing);
    if (bone == NULL || thing->GetPPos() == NULL) return false;

    m44 wpos = thing->GetPPos()->Rend.WorldPosition;
    v4 center = bone->SkinPoseMatrix.getCol3() + PORT_ZBIAS;

    v4 min = bone->BoundBoxMin;
    v4 max = bone->BoundBoxMax;

    PShape* part_shape = thing->GetPShape();
    if (part_shape != NULL)
    {
        center = wpos.getCol3() + PORT_ZBIAS;
        wpos = m44::identity();
        min = part_shape->Fork->Min;
        max = part_shape->Fork->Max;
    }

    if (thing->ObjectType == OBJECT_SWITCH_SELECTOR && (port != 0 || output))
    {
        PSwitch* sw = thing->GetPSwitch();


        float dist = abs(max.getX() - min.getX());

        float inc = dist / (float)sw->InputCount;

        if (!output) port -= 1;

        float left = min.getX() + (inc * (float)(port + 0));
        float right = min.getX() + (inc * (float)(port + 1));
        float top = max.getY();
        float bottom = min.getY();

        if (output)
            out = wpos * v4((left + right) / 2.0f, top + 11.331f, center.getZ(), 1.0f);
        else
            out = wpos * v4((left + right) / 2.0f, bottom - 11.331f, center.getZ(), 1.0f);

        return true;
    }

    int num_ports;
    if (output)
    {
        PSwitch* part_switch = thing->GetPSwitch();
        if (part_switch == NULL) return false;
        num_ports = part_switch->Outputs.size();
    }
    else 
    {
        num_ports = GetNumInputs(thing);
        if (thing->ObjectType == OBJECT_SWITCH_SELECTOR)
            num_ports = 1;
    }

    float top = max.getY();
    float bottom = max.getY();

    float dist = abs(max.getY() - min.getY());
    const float spacing = 61.8731f;
    if (num_ports > 2)
    {
        dist += (num_ports - 2) * (spacing * 0.5f);
        top += (num_ports - 2) * (spacing * 0.25f);
        bottom += (num_ports - 2) * (spacing * 0.25f);
    }

    float inc = dist / (float)num_ports;

    top -= (inc * (float)(port + 0));
    bottom -= (inc * (float)(port + 1));

    if (output)
        out = wpos * v4(max.getX() + PORT_RADIUS, ((top + bottom) / 2.0f), center.getZ(), 1.0f);
    else
        out = wpos * v4(min.getX() - PORT_RADIUS, ((top + bottom) / 2.0f), center.getZ(), 1.0f);
    
    return true;
}

void DrawWire(CSwitchOutput* output, CPoppet* player, CThing* target, int port)
{
    bool loose = false;
    

    v4 a, b;
    if (!GetPortPos(output->Owner->GetThing(), output->Port, true, a)) return;
    if (target != NULL && !GetPortPos(target, port, false, b)) return;

    if (target == NULL)
    {
        if (player != NULL)
            b = player->Raycast.GetHitPoint();
        else
            b = a + v4(5.0f, 0.0f, 0.0f, 0.0f);
        
        loose = true;
    }

    v4 points[8];
    v4 tint(1.0f);
    m44 trans = m44::identity();
    int num_points;
    float dist;

    if (loose)
    {
        num_points = 2;
        points[0] = a;
        points[1] = b;
        dist = Vectormath::Aos::length(b - a); 
    }
    else
    {
        bool below = a.getY() > b.getY();
        bool left = a.getX() > b.getX();

        // Hack for if we're connected to ourselves
        if (target == output->Owner->GetThing())
        {
            below = false;
            left = true;
        }

        num_points = Hermite(points, 7, a, b,
            500.0f,
            below ? -500.0f : (left ? 500.0f : 0.0f),
            500.0f,
            0.0f
        );

        points[num_points++] = b;

        dist = 0.0f;
        for (int i = 1; i < num_points; ++i)
            dist += Vectormath::Aos::length(points[i] - points[i - 1]);
    }

    CP<RGfxMaterial> mat = LoadResourceByKey<RGfxMaterial>(75684, 0, STREAM_PRIORITY_DEFAULT);
    mat->BlockUntilLoaded();

    RenderRope(
        mat,
        (const v2*)points,
        (const v2*)points,
        num_points,
        sizeof(v2),
        20.0f,
        dist,
        false,
        false,
        trans,
        trans,
        false,
        tint
    );
}

void CustomRenderWires()
{
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;

    // 0xcc7c - error wire
    // 0xadeb - default wire

    cellGcmSetDepthMask(gCellGcmCurrentContext, CELL_GCM_TRUE);

    // Render all wires that any player is holding

    CVector<SHeldWire> held_wires(world->ListPYellowHead.size());
    for (PYellowHead** it = world->ListPYellowHead.begin(); it != world->ListPYellowHead.end(); ++it)
    {
        PYellowHead* yellowhead = *it;
        if (yellowhead == NULL) continue;

        CPoppet* poppet = yellowhead->Poppet;
        if (poppet == NULL || poppet->GetSubMode() != SUBMODE_SWITCH_CONNECTOR) continue;

        if (poppet->Edit.SwitchConnectorRef == NULL) continue;
        held_wires.push_back(SHeldWire(poppet));

        DrawWire(
            poppet->Edit.SwitchConnectorRef->GetPSwitch()->Outputs[poppet->Edit.SwitchConnectorRefPort], 
            poppet, 
            poppet->Raycast.HitThing, 
            poppet->Raycast.HitPort
        );
    }

    // Render output wires
    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* part_switch = *it;
        if (part_switch == NULL) continue;
        CThing* thing = part_switch->GetThing();

        for (int i = 0; i < part_switch->Outputs.size(); ++i)
        {
            CSwitchOutput* output = part_switch->Outputs[i];
            
            // If we don't have any targets, render a small little output wire
            // to give the user indication that there's an output.
            if (output->TargetList.size() == 0)
            {
                // If anybody is holding a wire from this output, skip!
                bool has_wire = false;
                for (int i = 0; i < held_wires.size(); ++i)
                {
                    if (held_wires[i].Source == thing && held_wires[i].From == i)
                    {
                        has_wire = true;
                        break;
                    }
                }

                if (!has_wire)
                    DrawWire(output, NULL, NULL, -1);
            }

            for (int j = 0; j < output->TargetList.size(); ++j)
            {
                bool is_held_wire = false;
                CSwitchTarget& target = output->TargetList[j];

                for (SHeldWire* held_wire = held_wires.begin(); held_wire != held_wires.end(); ++held_wire)
                {
                    if (held_wire->Source == thing && held_wire->From == i && held_wire->To == target.Port && held_wire->Target == target.Thing)
                    {
                        is_held_wire = true;
                        break;
                    }
                }


                if (is_held_wire) continue;

                DrawWire(output, NULL, target.Thing, target.Port);
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

    CSwitchOutput* output = Edit.SwitchConnectorRef->GetPSwitch()->Outputs[Edit.SwitchConnectorRefPort];
    
    for (CThing** it = world->Things.begin(); it != world->Things.end(); ++it)
    {
        CThing* thing = (*it);
        if (thing == NULL || thing->GetPPos() == NULL || GetNumInputs(thing) == 0) continue;

        float t = 1.0e+20f;

        m44 wpos = thing->GetPPos()->Game.WorldPosition;
        int num_inputs = GetNumInputs(thing);

        for (int i = 0; i < num_inputs; ++i)
        {
            // If we're already connected to this port, we can ignore raycasting against it.
            if (thing->GetInput(i) == output) continue;

            if (!RaycastPort(thing, i, num_inputs, true, false, ray_start, ray_dir, t)) continue;
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

    float t = 1.0e+20f;
    CThing* hit;

    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* sw = (*it);
        CThing* thing = sw->GetThing();

        if (thing == ignored || thing->GetPPos() == NULL) continue;

        // Test against our own output ports first
        m44 wpos = thing->GetPPos()->Game.WorldPosition;
        for (int i = 0; i < sw->Outputs.size(); ++i)
        {
            if (!RaycastPort(thing, i, sw->Outputs.size(), false, true, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
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
                if (target->Thing == NULL || target->Thing->GetPPos() == NULL) continue;

                if (!RaycastPort(target->Thing, target->Port, GetNumInputs(target->Thing), false, false, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
                if (t >= 1.0e+20f) continue;

                raycast.SwitchConnector = true;
                raycast.HitThing = target->Thing;
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

#define MAX_RENDER_PORTS (512)
extern m44* HugeBoneArray;
CMeshInstance* HugePortArray;
int gRenderPortIndex;

void AddPortsToBuckets()
{
    for (int i = 0; i < MIN(gRenderPortIndex, MAX_RENDER_PORTS); ++i)
        HugePortArray[i].AddToBuckets(true, false, false);
}

void OnSceneGraphStartFrame()
{
    gRenderPortIndex = 0;
}

CMeshInstance* AllocateHugePort()
{
    return &HugePortArray[gRenderPortIndex++ % MAX_RENDER_PORTS];
}

void GatherPortsForDrawing()
{
    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;

    for (CThing** it = world->Things.begin(); it != world->Things.end(); ++it)
    {
        const CThing* thing = *it;
        if (thing == NULL) continue;

        int num_inputs = GetNumInputs(thing);
        if (num_inputs == 0) continue;

        CP<RMesh> primary_port_mesh;
        CP<RMesh> secondary_port_mesh;

        switch (thing->ObjectType)
        {
            case OBJECT_SWITCH_SELECTOR:
                primary_port_mesh = LoadResourceByKey<RMesh>(73813 , 0, STREAM_PRIORITY_DEFAULT);
                secondary_port_mesh = LoadResourceByKey<RMesh>(73762, 0, STREAM_PRIORITY_DEFAULT);

                break;
            case OBJECT_SWITCH_AND:
            case OBJECT_SWITCH_OR:
            case OBJECT_SWITCH_XOR:
                primary_port_mesh = LoadResourceByKey<RMesh>(73759, 0, STREAM_PRIORITY_DEFAULT);
                break;
            default:
                primary_port_mesh = LoadResourceByKey<RMesh>(73762, 0, STREAM_PRIORITY_DEFAULT);
                break;
        }
    
        switch (thing->ObjectType)
        {
            default:
            {
                if (thing->GetPPos() == NULL) continue;

                for (int i = 0; i < num_inputs; ++i)
                {
                    CP<RMesh> mesh = thing->ObjectType == OBJECT_SWITCH_SELECTOR && i == 0 ? secondary_port_mesh : primary_port_mesh;
                    if (!mesh->IsLoaded()) continue;

                    v4 port_pos;
                    if (!GetPortPos(thing, i, false, port_pos)) continue;
                    // port_pos -= PORT_ZBIAS;

                    CMeshInstance* inst = AllocateHugePort();

                    inst->FirstBoneIndex = AllocateHugeBones(1);
                    if (inst->FirstBoneIndex == -1) continue;
        

                    PRenderMesh dumb_hack;
                    dumb_hack.MeshInstance = inst;
                    dumb_hack.Mesh = mesh;
                    dumb_hack.DMACullBones();
        
                    inst->SetMesh(&mesh->mesh);
                    inst->Invalidate();

                    v4 translation; m44 rotation; v3 scale;
                    Decompose(translation, rotation, scale, thing->GetPPos()->Rend.WorldPosition);

                    // undo weird scaling shit
                    scale.setX(scale.getY());

                    // m44 wpos = rotation * m44::scale(scale);
                    m44 wpos = m44::identity();
                    wpos.setCol3(port_pos);

                    HugeBoneArray[inst->FirstBoneIndex] = wpos;
        
                    inst->InitInstanceForRender(inst->FirstBoneIndex, 1, NULL, NULL);
        
                    inst->InstanceTexture = NULL;
        
                    if (thing->ObjectType == OBJECT_SWITCH_SELECTOR)
                    {
                        if (i > 0)
                        {
                            float input = thing->GetPSwitch()->GetActivationFromInput(i).Analogue;
                            float output = thing->GetPSwitch()->Outputs[i - 1]->Activation.Analogue;
                            inst->InstanceColor = v4(abs(output), abs(input), 1.0f, 1.0f);
                        }
                    }
                    else
                    {
                        CSwitchOutput* signal = thing->GetInput(i);
                        if (signal == NULL) inst->InstanceColor = v4(0.0f, 0.0f, 0.0f, 1.0f);
                        else inst->InstanceColor = v4(abs(signal->Activation.Analogue));
                    }
                }

                break;
            }
        }


    }
}

MH_DefineFunc(CMeshInstance_AddToBuckets, 0x001f9328, TOC0, void, CMeshInstance*, bool, bool, bool);
void CMeshInstance::AddToBuckets(bool a, bool b, bool c)
{
    CMeshInstance_AddToBuckets(this, a, b, c);
}

MH_DefineFunc(_AllocateHugeBones, 0x001edf44, TOC0, u32, u32);
u32 AllocateHugeBones(u32 numBonesRequired)
{
    return _AllocateHugeBones(numBonesRequired);
}

MH_DefineFunc(CMeshInstance_CMeshInstance, 0x001ef184, TOC0, void, CMeshInstance*);
CMeshInstance::CMeshInstance()
{
    CMeshInstance_CMeshInstance(this);
}

MH_DefineFunc(CMeshInstance_Invalidate, 0x001f12a0, TOC0, void, CMeshInstance*);
void CMeshInstance::Invalidate()
{
    CMeshInstance_Invalidate(this);
}

MH_DefineFunc(CMeshInstance_SetMesh, 0x001f1708, TOC0, void, CMeshInstance*, CMesh*);
void CMeshInstance::SetMesh(CMesh* mesh)
{
    CMeshInstance_SetMesh(this, mesh);
}

MH_DefineFunc(CMeshInstance_InitInstanceForRender, 0x001edd98, TOC0, void, CMeshInstance*, u32, u32, CVector<void*>*, CVector<void*>*);
void CMeshInstance::InitInstanceForRender(u32 first_bone_index, u32 num_bones, CVector<void*>* decals, CVector<void*>* eyetoy)
{
    CMeshInstance_InitInstanceForRender(this, first_bone_index, num_bones, decals, eyetoy);
}

MH_DefineFunc(PRenderMesh_DMACullBones, 0x00045b9c, TOC0, void, const PRenderMesh*);
void PRenderMesh::DMACullBones() const
{
    PRenderMesh_DMACullBones(this);
}

void PRenderMesh::SetupRendering() const
{
    if (!Mesh || !Mesh->IsLoaded()) return;

    u32 first_bone_index = *(u32*)(((char*)MeshInstance) + 0x14c);

    const CThing* thing = GetThing();
    if (Mesh->GetGUID() == 73728 || Mesh->GetGUID() == 73733 || Mesh->GetGUID() == 73739)
    {
        const m44& wpos = thing->GetPPos()->Rend.WorldPosition;
        int input_count = thing->GetPSwitch()->InputCount;



        const float spacing = 61.8731f;
        float delta = spacing + (input_count - 2) * (spacing * 0.25f);
        
        HugeBoneArray[first_bone_index + 1] = wpos * m44::translation(v3(0.0f, delta, 0.0f)) * Mesh->mesh.Bones[1].InvSkinPoseMatrix;
        HugeBoneArray[first_bone_index + 2] = wpos * m44::translation(v3(0.0f, -delta, 0.0f)) * Mesh->mesh.Bones[2].InvSkinPoseMatrix;
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
extern "C" uintptr_t _render_wire_hook;

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
            GET_PREFIX(SELECTOR)
            GET_PREFIX(ANGLE)
        }

        #undef GET_PREFIX
        #undef GET_PREFIX_WITH_INVERSE

        if (prefix == NULL) return E_KEY_SWITCH;

        return MakeLamsKeyID(prefix, "_NAME");
    }

    void SetSwitchType(CThing* thing, int type)
    {
        PSwitch* part_switch;
        if (thing == NULL || (part_switch = thing->GetPSwitch()) == NULL) return;

        part_switch->Type == type;
        thing->OnFixup();
    }

    void Register()
    {
        // RegisterNativeFunction("SwitchBase", "GetSwitchType__", false, NVirtualMachine::CNativeFunction1<int, CThing*>::Call<GetSwitchType>);
        // RegisterNativeFunction("SwitchBase", "GetNumOutputs__", false, NVirtualMachine::CNativeFunction1<int, CThing*>::Call<GetNumOutputs>);
        // RegisterNativeFunction("SwitchBase", "GetTweakTitle__", false, NVirtualMachine::CNativeFunction1<int, CThing*>::Call<GetTweakTitle>);
        // RegisterNativeFunction("SwitchBase", "SetSwitchType__i", false, NVirtualMachine::CNativeFunction2V<CThing*, int>::Call<SetSwitchType>);
    }
}

void InitLogicSystemHooks()
{
    LogicSystemNativeFunctions::Register();

#ifdef __NEW_LOGIC_SYSTEM__
    HugePortArray = (CMeshInstance*)CAllocatorMMAligned128::Malloc(gOtherBucket, sizeof(CMeshInstance) * MAX_RENDER_PORTS);
    for (int i = 0; i < MAX_RENDER_PORTS; ++i)
        new (HugePortArray + i) CMeshInstance();

    MH_InitHook((void*)0x007716e8, (void*)&GatherPoppetRaycastVariables);
    MH_InitHook((void*)0x00353370, (void*)&CustomRaycastAgainstSwitches);
    MH_PokeMemberHook(0x00365a0c, CPoppetEditState::SetSwitchConnector);
    MH_PokeMemberHook(0x003759dc, CPoppetEditState::UpdateSwitchConnector);
    MH_PokeMemberHook(0x0034aa24, CPoppet::RaycastAgainstSwitchConnector);
    MH_PokeMemberHook(0x00078ed8, PSwitch::Update);
    MH_PokeMemberHook(0x00077c18, PEmitter::Update);

    MH_PokeBranch(0x001df8f4, &_render_wire_hook);
    MH_PokeBranch(0x0003ecb4, &_render_mesh_setup_rendering_hook);


    MH_PokeCall(0x001e9f3c, OnSceneGraphStartFrame);
    MH_PokeCall(0x00014040, OnSceneGraphStartFrame);

    MH_PokeHook(0x001c0fac, AddPortsToBuckets);
    MH_PokeHook(0x001c719c, GatherPortsForDrawing);

    MH_PokeHook(0x007407b4, GatherSwitchVariables);
#endif

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