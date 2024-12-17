#include "sm64/avatar.h"

#include <cell/gcm.h>

#include <cell/DebugLog.h>
#include <mmalex.h>

#include <ResourceSystem.h>
#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <ResourcePlan.h>
#include <PartPhysicsWorld.h>
#include <PartRenderPosition.h>
#include <PartRenderMesh.h>
#include <Mesh.h>
#include <PartGeneratedMesh.h>

#include <Poppet.h>
#include <GFXApi.h>

#include <network/NetworkManager.h>

inline float UnpackAnalogue(u16 raw)
{
    float val = (int)(raw - 128);
    if (abs(val) < 37.0f) return 0.0f;
    return val / 128.0f;
}

CVector<CMarioAvatar*> gMarioAvatars;
bool gMarioInputEnabled;

const u32 E_KEY_MARIO_MESH = 2673761222u;
const u32 E_KEY_MARIO_OBJ = 4244711528u;
const u32 E_KEY_MARIO_OBJ_NO_PHYS = 2954560587u;

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

MH_DefineFunc(DeleteGroupAndChildrenAndJoints, 0x003f6eec, TOC1, void, CThing*);
CMarioAvatar::~CMarioAvatar()
{
    if (Thing != NULL) DeleteGroupAndChildrenAndJoints(Thing);
    
    if (IsValid()) sm64_mario_delete(Id);

    for (CMarioThing* it = SimObjects.begin(); it != SimObjects.end(); ++it)
    {
        if (it->Id != -1)
            sm64_surface_object_delete(it->Id);
        it->Id = -1;
    }

    EPlayerNumber leader = gNetworkManager.InputManager.GetLocalLeadersPlayerNumber();
    CThing* sackboy = gGame->GetYellowheadFromPlayerNumber(leader);
    if (sackboy != NULL)
        sackboy->GetPRenderMesh()->Visible = true;
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

bool CMarioAvatar::IsShapeNearby(CThing* thing)
{
    v4 bb_min = MarioSearchMin;
    v4 bb_max = MarioSearchMax;

    if (thing == NULL) return false;

    PPos* pos = thing->GetPPos();
    PShape* shape = thing->GetPShape();
    if (shape == NULL || pos == NULL) return false;

    m44& wpos = pos->Game.WorldPosition;

    v4 thing_min(INFINITY);
    v4 thing_max(-INFINITY);

    for (v4* it = (v4*)shape->Polygon.begin(); it != (v4*)shape->Polygon.end(); ++it)
    {
        v4 v = wpos * (*it);

        thing_min = Vectormath::Aos::minPerElem(thing_min, v);
        thing_max = Vectormath::Aos::maxPerElem(thing_max, v);
    }

    // v4 thing_min = *((v4*)&shape->Game.Min);
    // v4 thing_max = *((v4*)&shape->Game.Max);

    return
        thing_min.getX() > bb_min.getX() &&
        thing_min.getY() > bb_min.getY() &&
        thing_min.getZ() > bb_min.getZ() &&

        thing_max.getX() < bb_max.getX() &&
        thing_max.getY() < bb_max.getY() &&
        thing_max.getZ() < bb_max.getZ();
}

void CMarioAvatar::UpdateSimObjects()
{
    CMarioThing* it = SimObjects.begin();
    while (it != SimObjects.end())
    {
        if (it->ShouldDestroy() || !IsShapeNearby(it->Thing))
        {
            DebugLogChF(DC_SM64, "Removing object id %d from world since thing no longer exists in world\n", it->Id);
            sm64_surface_object_delete(it->Id);
            it = SimObjects.erase(it);

            continue;
        }

        it->UpdatePosition();

        it++;
    }
}

bool CMarioAvatar::IsMarioThing(CThing* thing)
{
    if (Thing == NULL || Thing->GroupHead == NULL) return false;
    PGroup* group = Thing->GroupHead->GetPGroup();
    if (group == NULL) return false;
    return group->HasMember(thing);
}

MH_DefineFunc(PShape_InitialisePolygon, 0x0005c7e0, TOC0, void, PShape*);
MH_DefineFunc(TriangulateByEarClipping, 0x005d3c7c, TOC0, void, CRawVector<v2, CAllocatorMMAligned128>& polygon, CRawVector<unsigned int>& loops, CRawVector<unsigned int>& triangles, bool flipped);
void CMarioAvatar::UpdateWorld()
{
    if (Thing == NULL) return;
    UpdateSimObjects();

    PWorld* world = Thing->World;
    // sm64_set_mario_water_level(Id, (s32)world->GetWaterLevelWorldYCoords());

    for (CThing** it = world->Things.begin(); it != world->Things.end(); ++it)
    {
        CThing* thing = (*it);
        if (thing == NULL || IsMarioThing(thing)) continue;

        PPos* part_pos = thing->GetPPos();
        PShape* part_shape = thing->GetPShape();
        PBody* part_body = thing->GetPBody();

        if (part_pos == NULL || part_shape == NULL || thing->BodyRoot == NULL) continue;
        
        if (!IsShapeNearby(thing)) continue;


        CP<RMaterial>& material = part_shape->MMaterial;
        if (!material) continue;

        CGUID& guid = material->GetGUID();
        if (
            guid == 11455 || guid == 3247 || guid == 8390 || guid == 17661 || 
            guid == 66189 || guid == 66206 || guid == 30622 || guid == 6448) continue;

        if (SimObjectExists(thing->UID)) continue;
        if (part_shape->Polygon.size() == 0) continue;

        m44 wpos = part_pos->Game.WorldPosition;
        v4 pos = wpos.getCol3();

        int num_verts = part_shape->Polygon.size();
        v4* polygon = (v4*)part_shape->Game.GlobalPolygon.begin();

        float scale_x = Vectormath::Aos::length(wpos.getCol0());
        float scale_y = Vectormath::Aos::length(wpos.getCol1());
        float scale_z = Vectormath::Aos::length(wpos.getCol2());
        bool is_flipped = Vectormath::Aos::determinant(wpos) < 0.0f;
        is_flipped = false;

        v3 scale(scale_x, scale_y, scale_z);
        scale *= is_flipped ? -1.0f : 1.0f;
        
        m44 wpos_scalerot = m44::scale(scale);
        m44 transform = wpos_scalerot;

        CRawVector<v4, CAllocatorMMAligned128> vertices(num_verts * 2);
        for (int i = 0; i < num_verts; ++i)
        {
            // v4 v = transform * polygon[i];
            v4 v = polygon[i] - pos;

            v.setZ(part_shape->Thickness);
            v.setW(1.0f);

            vertices.push_back(v);
        }

        for (int i = 0; i < num_verts; ++i)
        {
            // v4 v = transform * polygon[i];
            v4 v = polygon[i] - pos;

            v.setZ(-part_shape->Thickness);
            v.setW(1.0f);
            vertices.push_back(v);
        }

        #define PUSH_3(x, y, z) \
        if (is_flipped) \
        { \
            triangles.push_back(x); triangles.push_back(z); triangles.push_back(y); \
        } \
        else \
        { \
            triangles.push_back(x); triangles.push_back(y); triangles.push_back(z); \
        }
        
        CRawVector<unsigned int> triangles(num_verts * 2 * 3);
        int offset = 0, front = 0, back = num_verts;
        for (unsigned int* it = part_shape->Loops.begin(); it != part_shape->Loops.end(); ++it)
        {
            unsigned int loop = *it;
            for (int i = offset; i < offset + loop - 1; ++i)
            {
                PUSH_3((i + 1) + back, i + back, i);
                PUSH_3((i + 1) + back, i, i + 1);
            }

            PUSH_3(offset, offset + loop + back - 1, offset + loop - 1);
            PUSH_3(offset, offset + back, offset + loop + back - 1);

            offset += loop;
        }
        #undef PUSH_3

        CRawVector<unsigned int> indices;
        TriangulateByEarClipping(part_shape->Polygon, part_shape->Loops, indices, true);
        for (int i = 0; i < indices.size(); ++i)
            triangles.push_back(indices[i]);
        
        CMarioThing obj;
        SM64SurfaceObject surface_obj;
        memset(&surface_obj, 0, sizeof(SM64SurfaceObject));

        obj.Thing = thing;

        surface_obj.transform.position[0] = pos.getX();
        surface_obj.transform.position[1] = pos.getY();
        surface_obj.transform.position[2] = pos.getZ();

        obj.LastPosition[0] = (s16)surface_obj.transform.position[0];
        obj.LastPosition[1] = (s16)surface_obj.transform.position[1];
        obj.LastPosition[2] = (s16)surface_obj.transform.position[2];

        const float rad2deg = 360.0f / (M_PI * 2.0f);
        float angle = GetWorldAngle(Thing);
        if (angle < -M_PI) angle += M_PI * 2.0f;
        if (angle > M_PI) angle -= M_PI * 2.0f;
        angle *= rad2deg;
        // surface_obj.transform.eulerRotation[2] = angle;




        surface_obj.surfaceCount = triangles.size() / 3;
        surface_obj.surfaces = new SM64Surface[surface_obj.surfaceCount];

        for (int i = 0; i < surface_obj.surfaceCount; ++i)
        {
            SM64Surface& surface = surface_obj.surfaces[i];
            surface.type = SURFACE_DEFAULT;
            surface.force = 0;
            surface.terrain = TERRAIN_GRASS;

            for (int j = 0; j < 3; ++j)
            {
                u16 face = triangles[(i * 3) + j];

                v4 v = vertices[face];

                surface.vertices[j][0] = (s32)((float)v.getX());
                surface.vertices[j][1] = (s32)((float)v.getY());
                surface.vertices[j][2] = (s32)((float)v.getZ());
            }
        }

        obj.Id = sm64_surface_object_create(&surface_obj);

        delete[] surface_obj.surfaces;
        surface_obj.surfaces = 0;
        surface_obj.surfaceCount = 0;

        SimObjects.push_back(obj);
        DebugLogChF(DC_SM64, "physobj thing %d -> simobject %d, num_verts=%d, num_tris=%d, rotation=%f, baked_scale=[%f, %f, %f]\n",
            thing->UID, obj.Id,
            num_verts,
            triangles.size() / 3,
            angle,
            scale_x, scale_y, scale_z
        );
    }
}

void CMarioAvatar::DoDebugRender()
{

}

void CMarioAvatar::InitThing()
{
    PWorld* world;
    CThing* world_thing = gGame->Level->WorldThing;
    if (world_thing == NULL || (world = world_thing->GetPWorld()) == NULL) return;

    CP<RPlan> stub = LoadResourceByKey<RPlan>(E_KEY_MARIO_OBJ_NO_PHYS, 0, STREAM_PRIORITY_DEFAULT);
    stub->BlockUntilLoaded();

    NetworkPlayerID default_id;
    memset(&default_id, 0, sizeof(NetworkPlayerID));
    StringCopy<char, 16>(default_id.handle.data, "mold");
    Thing = RPlan::MakeClone(stub, world, default_id, true);
    Thing->GetPRenderMesh()->Mesh = Mesh;

    PGroup* group = Thing->GroupHead->GetPGroup();
    for (CThing** it = group->GroupMemberList.begin(); it != group->GroupMemberList.end(); ++it)
    {
        CThing* thing = *it;
        if (thing->GetPShape() != NULL && thing != Thing.GetThing())
            PhysicsThing = thing;
    }
}

#include <mario_animation_ids.h>
void CMarioAvatar::Tick()
{
    if (!IsValid()) return;
    if (Thing != NULL)
    {
        v4 mario_radius = v4(3000.0f);
        v4 mario_pos = Thing->GetPPos()->GetBestGameplayPosv4();
        MarioSearchMin = mario_pos - mario_radius;
        MarioSearchMax = mario_pos + mario_radius;
    }

    UpdateInput();
    UpdateWorld();

    sm64_mario_tick(Id, &Inputs, &State, &Geometry);

    UpdateMesh();
    // UpdateThing();
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
    Inputs.buttonA = (Pad->Buttons & PAD_BUTTON_CROSS) != 0;
    Inputs.buttonB = (Pad->Buttons & PAD_BUTTON_SQUARE) != 0;
    Inputs.buttonZ = (Pad->Buttons & PAD_BUTTON_L2) != 0;
    
    sm64_set_sound_volume(100.0f);
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

    m44 pm = m44::identity();
    pm.setCol3(v4(State.position[0], State.position[1] + 28.7866f, State.position[2], 1.0f));

    if (PhysicsThing.GetThing() != NULL)
        PhysicsThing->GetPPos()->SetWorldPos(pm, true, 0);
    pos->SetWorldPos(m, false, 0);

    EPlayerNumber leader = gNetworkManager.InputManager.GetLocalLeadersPlayerNumber();
    CThing* sackboy = gGame->GetYellowheadFromPlayerNumber(leader);
    if (sackboy != NULL)
    {
        sackboy->GetPRenderMesh()->Visible = false;

        v2 pos(State.position[0], State.position[1], State.position[2], 1.0f);
        // gGame->TeleportPlayer(sackboy, pos);
        sackboy->GetPPos()->SetWorldPos(m, false, 0);
    }
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