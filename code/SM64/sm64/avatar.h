#ifndef SM64_AVATAR_H
#define SM64_AVATAR_H

#include <libsm64.h>
#include <sm64.h>

#include <vector.h>
#include <refcount.h>
#include <thing.h>
#include <padinput.h>

#include <cell/DebugLog.h>

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
    #define CONVERT_ANGLE( x ) ((s16)( -(x) / 180.0f * 32768.0f ))
    inline bool UpdatePosition()
    {
        if (!IsValid()) return false;

        PPos* part_pos = Thing->GetPPos();
        if (part_pos == NULL) return false;

        m44 wpos = part_pos->Game.WorldPosition;
        v4 pos = wpos.getCol3();

        s16 posx = (s16)pos.getX();
        s16 posy = (s16)pos.getY();
        s16 posz = (s16)pos.getZ();

        s16& old_posx = LastPosition[0];
        s16& old_posy = LastPosition[1];
        s16& old_posz = LastPosition[2];
        s16& old_angle = LastRotation;

        const float rad2deg = 360.0f / (M_PI * 2.0f);
        float angle = GetWorldAngle(Thing);
        if (angle < -M_PI) angle += M_PI * 2.0f;
        if (angle > M_PI) angle -= M_PI * 2.0f;
        angle *= rad2deg;

        s16 new_angle = CONVERT_ANGLE(angle);
        
        if (old_posx != posx || old_posy != posy || old_posz != posz || old_angle != new_angle)
        {
            SM64ObjectTransform transform;
            memset(&transform, 0, sizeof(SM64ObjectTransform));

            transform.position[0] = pos.getX();
            transform.position[1] = pos.getY();
            transform.position[2] = pos.getZ();
            // transform.eulerRotation[2] = angle;

            DebugLogChF(DC_SM64, "updating physobj: %d, pos: [%f, %f, %f], rot: %f\n", Id, (float)pos.getX(), (float)pos.getY(), (float)pos.getZ(), angle);

            LastPosition[0] = posx;
            LastPosition[1] = posy;
            LastPosition[2] = posz;
            LastRotation = new_angle;

            sm64_surface_object_move(Id, &transform);

            return true;
        }

        return false;
    }
    #undef CONVERT_ANGLE

    inline bool ShouldDestroy()
    {
        if (!IsValid()) return true;
        return Thing->GetPShape() == NULL;
    }
    
    inline bool IsValid() { return Id != -1 && Thing != NULL; }
public:
    s16 LastPosition[3];
    s16 LastRotation;
    CThingPtr Thing;
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
    void UpdateThing();
    void DoDebugRender();
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
    
    void UpdateSimObjects();
    bool SimObjectExists(u32 uid);
    bool IsShapeNearby(CThing* thing);
    bool IsMarioThing(CThing* thing);
public:
    SM64MarioInputs Inputs;
    SM64MarioState State;
    SM64MarioGeometryBuffers Geometry;
private:
    s32 Id;
    PadData* Pad;
    CThingPtr Thing;
    CThingPtr PhysicsThing;
    CP<RMesh> Mesh;
    float CameraRotation;
    float CameraPos[3];
    CDebugCamera Camera;
    CVector<CMarioThing> SimObjects;
    v4 MarioSearchMin;
    v4 MarioSearchMax;
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