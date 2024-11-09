#ifndef SM64_AVATAR_H
#define SM64_AVATAR_H

#include <libsm64.h>
#include <sm64.h>

#include <vector.h>
#include <refcount.h>
#include <thing.h>
#include <padinput.h>

#include <DebugCamera.h>
#include <ResourceGFXMesh.h>

class CMarioThing {
public:
    inline CMarioThing()
    {
        memset(this, 0, sizeof(CMarioThing));
        Id = -1;
    }
public:
    inline bool UpdatePosition()
    {
        if (ShouldDestroy()) return false;
        
        PPos* part_pos = Thing->GetPPos();
        if (part_pos == NULL) return false;

        m44 wpos = part_pos->Game.WorldPosition;
        v4 pos = wpos.getCol3();

        s32 posx = (s32)pos.getX();
        s32 posy = (s32)pos.getY();
        s32 posz = (s32)pos.getZ();
        
        s32 old_posx = (s32)Object.transform.position[0];
        s32 old_posy = (s32)Object.transform.position[1];
        s32 old_posz = (s32)Object.transform.position[2];

        if (old_posx != posx || old_posy != posy || old_posz != posz)
        {
            Object.transform.position[0] = pos.getX();
            Object.transform.position[1] = pos.getY();
            Object.transform.position[2] = pos.getZ();

            sm64_surface_object_move(Id, &Object.transform);

            return true;
        }

        return false;
    }

    inline bool ShouldDestroy()
    {
        if (!IsValid()) return true;

        PGeneratedMesh* part = Thing->GetPGeneratedMesh();
        if (part == NULL) return true;

        return part->SharedMesh == NULL;
    }
    
    inline bool IsValid() { return Id != -1 && Thing != NULL; }
public:
    CThingPtr Thing;
    SM64SurfaceObject Object;
    s32 Id;
};

class CMarioAvatar {
public:
    CMarioAvatar(EPadIndex pad, float x, float y, float z);
    ~CMarioAvatar();
public:
    inline s32 GetID() { return Id; }
    inline bool IsValid() { return Id != -1; }
public:
    void Tick();
    inline void ApplyCamera(CCamera* camera)
    {
        Camera.Apply(camera);
    }
private:
    void InitThing();
    void InitMesh();


    void UpdateWorld();
    void UpdateInput();
    void UpdateMesh();
    void UpdateThing();
    
    void UpdateSimObjects();
    bool SimObjectExists(u32 uid);
public:
    SM64MarioInputs Inputs;
    SM64MarioState State;
    SM64MarioGeometryBuffers Geometry;
private:
    s32 Id;
    PadData* Pad;
    CThingPtr Thing;
    CP<RMesh> Mesh;
    float CameraRotation;
    float CameraPos[3];
    CDebugCamera Camera;
    CVector<CMarioThing> SimObjects;
    bool Paused;
private:
    float Position[9 * SM64_GEO_MAX_TRIANGLES];
    float Color[9 * SM64_GEO_MAX_TRIANGLES];
    float Normal[9 * SM64_GEO_MAX_TRIANGLES];
    float UV[6 * SM64_GEO_MAX_TRIANGLES];
};

extern CVector<CMarioAvatar*> gMarioAvatars;
extern bool gMarioInputEnabled;

#endif // SM64_AVATAR_H