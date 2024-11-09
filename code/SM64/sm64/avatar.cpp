#include "sm64/avatar.h"

#include <cell/DebugLog.h>
#include <mmalex.h>

#include <ResourceSystem.h>
#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <PartPhysicsWorld.h>
#include <PartRenderPosition.h>
#include <PartRenderMesh.h>
#include <Mesh.h>
#include <PartGeneratedMesh.h>

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

    for (CMarioThing* it = SimObjects.begin(); it != SimObjects.end(); ++it)
    {
        sm64_surface_object_delete(it->Id);
        delete[] it->Object.surfaces;
    }
}

void CMarioAvatar::InitMesh()
{
    CP<RMesh> stub = LoadResourceByKey<RMesh>(E_KEY_MARIO_MESH, 0, STREAM_PRIORITY_DEFAULT);
    stub->BlockUntilLoaded();
    CP<RMesh> mesh = AllocateNewResource<RMesh>(FLAGS_TEMPORARY);
    if (mesh->Duplicate(stub) == REFLECT_OK)
        Mesh = mesh;
}

bool CMarioAvatar::SimObjectExists(u32 uid)
{
    for (CMarioThing* it = SimObjects.begin(); it != SimObjects.end(); ++it)
    {
        if (it->IsValid() && it->Thing->UID == uid) 
            return true;
    }

    return false;
}

void CMarioAvatar::UpdateSimObjects()
{
    CMarioThing* it = SimObjects.begin();
    while (it != SimObjects.end())
    {
        if (it->ShouldDestroy())
        {
            DebugLogChF(DC_SM64, "Removing object id %d from world since thing no longer exists in world\n", it->Id);
            sm64_surface_object_delete(it->Id);
            delete[] it->Object.surfaces;

            it = SimObjects.erase(it);

            continue;
        }

        it->UpdatePosition();
        
        PGeneratedMesh* part_mesh = Thing->GetPGeneratedMesh();
        if (part_mesh != NULL && part_mesh->SharedMesh == NULL)
        {
            DebugLog("WARNING: %d is no longer visible, but we're still calculating it!\n", it->Id);
        }

        it++;
    }
}

void CMarioAvatar::UpdateWorld()
{
    if (Thing == NULL) return;
    UpdateSimObjects();
    

    PWorld* world = Thing->World;
    for (PGeneratedMesh** it = world->ListPGeneratedMesh.begin(); it != world->ListPGeneratedMesh.end(); ++it)
    {
        CThing* thing = (*it)->GetThing();
        if (thing == NULL) continue;

        PGeneratedMesh* part_mesh = thing->GetPGeneratedMesh();
        PPos* part_pos = thing->GetPPos();
        PShape* part_shape = thing->GetPShape();
        if (part_pos == NULL || part_shape == NULL || part_mesh == NULL) continue;

        if (!part_mesh->WasJustVisible) continue;
        if (SimObjectExists(thing->UID)) continue;

        m44 wpos = part_pos->Game.WorldPosition;
        v4 pos = wpos.getCol3();

        m33 com3 = wpos.getUpper3x3();
        m44 com = m44::identity();
        com = com.setUpper3x3(com3);
        m44 inv_com = com;
    

        inv_com = inv_com.setCol3(v4(0.0f, 0.0f, 0.0f, 1.0f));

        const CMesh* mesh = part_mesh->SharedMesh;
        if (mesh == NULL) continue;

        float* vertices = (float*)((char*)mesh->SourceGeometry.CachedAddress + mesh->SourceStreamOffsets[0]);
        u16* indices = (u16*)(mesh->Indices.CachedAddress);

        CRawVector<u16> triangles((mesh->NumVerts - 2) * 3);
        triangles.push_back(indices[0]);
        triangles.push_back(indices[1]);
        triangles.push_back(indices[2]);
        for (int i = 3, j = 1; i < mesh->NumIndices; ++i, ++j)
        {
            if (indices[i] == 65535)
            {
                if (i + 3 >= mesh->NumIndices) break;

                triangles.push_back(indices[i + 1]);
                triangles.push_back(indices[i + 2]);
                triangles.push_back(indices[i + 3]);

                i += 3;
                j = 0;
                continue;
            }

            if ((j & 1) != 0)
            {
                triangles.push_back(indices[i - 2]);
                triangles.push_back(indices[i]);
                triangles.push_back(indices[i - 1]);
            }
            else
            {
                triangles.push_back(indices[i - 2]);
                triangles.push_back(indices[i - 1]);
                triangles.push_back(indices[i]);
            }
        }

        CMarioThing obj;

        obj.Thing = thing;

        obj.Object.transform.position[0] = pos.getX();
        obj.Object.transform.position[1] = pos.getY();
        obj.Object.transform.position[2] = pos.getZ();

        obj.Object.surfaceCount = triangles.size() / 3;
        obj.Object.surfaces = new SM64Surface[obj.Object.surfaceCount];

        for (int i = 0; i < obj.Object.surfaceCount; ++i)
        {
            SM64Surface& surface = obj.Object.surfaces[i];
            surface.type = SURFACE_DEFAULT;
            surface.force = 0;
            surface.terrain = TERRAIN_GRASS;

            for (int j = 0; j < 3; ++j)
            {
                u16 face = triangles[(i * 3) + j];

                float* vertex = vertices + (face * 4);

                v4 v(vertex[0], vertex[1], vertex[2], 1.0f);
                v = inv_com * v;

                surface.vertices[j][0] = (s32)((float)v.getX());
                surface.vertices[j][1] = (s32)((float)v.getY());
                surface.vertices[j][2] = (s32)((float)v.getZ());
            }
        }

        obj.Id = sm64_surface_object_create(&obj.Object);
        SimObjects.push_back(obj);
        DebugLogChF(DC_SM64, "Adding SM64Object with %d vertices, and %d indices, and %d triangles\n", mesh->NumVerts, triangles.size(), triangles.size() / 3);
    }
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
    UpdateWorld();

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

    if (Pad->ButtonsDown & PAD_BUTTON_TRIANGLE)
        sm64_mario_interact_cap(Id, MARIO_WING_CAP, 0, false);

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