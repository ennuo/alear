#ifndef SM64_AVATAR_H
#define SM64_AVATAR_H

#include <libsm64.h>
#include <vector.h>
#include <refcount.h>
#include <thing.h>
#include <padinput.h>

#include <DebugCamera.h>
#include <ResourceGFXMesh.h>

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

    void UpdateInput();
    void UpdateMesh();
    void UpdateThing();
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