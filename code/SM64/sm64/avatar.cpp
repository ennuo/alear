#include "sm64/avatar.h"

#include <cell/DebugLog.h>
#include <mmalex.h>

#include <ResourceSystem.h>
#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <PartPhysicsWorld.h>
#include <PartRenderPosition.h>
#include <PartRenderMesh.h>

inline float UnpackAnalogue(u16 raw)
{
    float val = (int)(raw - 128);
    if (abs(val) < 37.0f) return 0.0f;
    return val / 128.0f;
}

CVector<CMarioAvatar*> gMarioAvatars;
bool gMarioInputEnabled;

const u32 E_KEY_MARIO_MESH = 2673761222u;

CMarioAvatar::CMarioAvatar(EPadIndex pad, float x, float y, float z)
{
    memset(this, 0, sizeof(CMarioAvatar));
    Geometry.position = Position;
    Geometry.color = Color;
    Geometry.normal = Normal;
    Geometry.uv = UV;

    Pad = gPadData + pad;
    Id = sm64_mario_create(x, y, z);

    InitMesh();
    InitThing();
}

CMarioAvatar::~CMarioAvatar()
{
    if (Thing.GetThing() != NULL)
        delete Thing.GetThing();
    if (IsValid())
        sm64_mario_delete(Id);
}

void CMarioAvatar::InitMesh()
{
    CP<RMesh> stub = LoadResourceByKey<RMesh>(E_KEY_MARIO_MESH, 0, STREAM_PRIORITY_DEFAULT);
    stub->BlockUntilLoaded();
    CP<RMesh> mesh = AllocateNewResource<RMesh>(FLAGS_TEMPORARY);
    if (mesh->Duplicate(stub) == REFLECT_OK)
        Mesh = mesh;
}

void CMarioAvatar::InitThing()
{
    PWorld* world;
    CThing* world_thing = gGame->Level->WorldThing;
    if (world_thing == NULL || (world = world_thing->GetPWorld()) == NULL) return;

    Thing = new CThing();
    Thing->SetWorld(world, 0);
    Thing->AddPart(PART_TYPE_POS);
    Thing->AddPart(PART_TYPE_RENDER_MESH);

    PPos* pos = Thing->GetPPos();
    if (pos != NULL)
        pos->ThingOfWhichIAmABone = Thing;

    PRenderMesh* render_mesh = Thing->GetPRenderMesh();
    if (render_mesh != NULL)
    {
        render_mesh->Mesh = Mesh;
        render_mesh->BoneThings.push_back(Thing);
    }
}

void CMarioAvatar::Tick()
{
    if (!IsValid()) return;
    
    UpdateInput();

    sm64_mario_tick(Id, &Inputs, &State, &Geometry);

    UpdateMesh();
    UpdateThing();
}

void CMarioAvatar::UpdateInput()
{
    if (Pad == NULL || Paused)
    {
        memset(&Inputs, 0, sizeof(SM64MarioInputs));
        return;
    }

    Inputs.stickX = UnpackAnalogue(Pad->LeftStickX);
    Inputs.stickY = UnpackAnalogue(Pad->LeftStickY);
    Inputs.buttonA = (Pad->Buttons & PAD_BUTTON_CIRCLE) != 0;
    Inputs.buttonB = (Pad->Buttons & PAD_BUTTON_SQUARE) != 0;
    Inputs.buttonZ = (Pad->Buttons & PAD_BUTTON_L1) != 0;

    float x0 = UnpackAnalogue(Pad->RightStickX);
    CameraRotation += x0 * (1.0f / 30.0f) * 2.0f;
    CameraPos[0] = State.position[0] + 1000.0f * mmalex::cos(CameraRotation);
    CameraPos[1] = State.position[1] + 200.0f;
    CameraPos[2] = State.position[2] + 1000.0f * mmalex::sin(CameraRotation);

    Inputs.camLookX = Position[0] - CameraPos[0];
    Inputs.camLookZ = Position[2] - CameraPos[2];
    
    Camera.Pos = v4(CameraPos[0], CameraPos[1], CameraPos[2], 1.0f);
    Camera.Foc = v4(State.position[0], State.position[1], State.position[2], 1.0f);
}

void CMarioAvatar::UpdateThing()
{
    if (Thing.GetThing() == NULL) return;
    PPos* pos = Thing->GetPPos();
    if (pos == NULL) return;

    m44 m = m44::identity();
    m.setCol3(v4(State.position[0], State.position[1], State.position[2], 1.0f));
    pos->SetWorldPos(m, false, 0);
}

void CMarioAvatar::UpdateMesh()
{
    if (!Mesh) return;

    CMesh& mesh = Mesh->mesh;
    CPrimitive& primitive = mesh.Primitives.front();

    int indices_used = Geometry.numTrianglesUsed * 3;

    char* posbuf = (char*)mesh.SourceGeometry.CachedAddress + mesh.SourceStreamOffsets[0];
    char* skinbuf = (char*)mesh.SourceGeometry.CachedAddress + mesh.SourceStreamOffsets[1];
    char* attrbuf = (char*)mesh.AttributeData.CachedAddress;

    float* vertices = (float*)posbuf;
    float* uvs = (float*)attrbuf;
    char* normals = skinbuf + 0x4;
    char* smooth_normals = skinbuf + 0xc;

    for (int i = 0; i < indices_used; ++i, vertices += 4, uvs += mesh.AttributeCount * 2, normals += 16, smooth_normals += 16)
    {
        vertices[0] = *(Geometry.position + (i * 3) + 0) - State.position[0];
        vertices[1] = *(Geometry.position + (i * 3) + 1) - State.position[1];
        vertices[2] = *(Geometry.position + (i * 3) + 2) - State.position[2];

        uvs[0] = *(Geometry.uv + (i * 2) + 0);
        uvs[1] = *(Geometry.uv + (i * 2) + 1);

        // sneak color into secondary and decal uv
        uvs[2] = *(Geometry.color + (i * 3) + 0);
        uvs[3] = *(Geometry.color + (i * 3) + 1);
        uvs[4] = *(Geometry.color + (i * 3) + 2);
        uvs[5] = 1.0f;

        // pack normal into 24 bits
        float* normal = Geometry.normal + (i * 3);
        int nx = (int)(normal[0] * 0x7ff) & 0xfff;
        int ny = (int)(normal[1] * 0x3ff) & 0x7ff;
        int nz = (normal[2] < 0) ? 1 : 0;
        int n = (nx | (ny << 12) | (nz << 23));

        normals[0] = n >> 16;
        normals[1] = n >> 8;
        normals[2] = n & 0xff;

        smooth_normals[0] = normals[0];
        smooth_normals[1] = normals[1];
        smooth_normals[2] = normals[2];
    }

    primitive.MaxVert = indices_used;
    primitive.NumIndices = indices_used;
}