#ifndef VIEW_H
#define VIEW_H

#include "Camera.h"
#include "DebugCamera.h"
#include "PartPhysicsWorld.h"

#include "vector.h"

// 0x350 = dof_min
// 0x354 = dof_max
// 0x530 = CDebugCamera
// 0x590 = DebugCameraActive



class CView {
public:
    CCamera* Camera;
    CCamera SunCamera;
    CCamera _OtherCameraIGuess;
    float DofMin;
    float DofMax;
    float FogMin;
    float FogMax;
    v2 MousePos;
    v2 MouseDir;
    v2 MousePosScreen;
    u32 LevelSettingsColorHash;
    CRawVector<PWorld*> WorldList;
    bool RenderExtras;
    bool UpdateRaycast;
    bool OutputToPodComputerRT;
    bool TransparentBG;
    bool DisableBorders;
    bool WaitForICacheUpdate;
    CDebugCamera PresetCams[4];
    CDebugCamera DebugCamera;
    bool DebugCameraActive;
    bool DebugCameraViewOnly;
    bool MaverickCam;
    CCamera RendCamera;
};

extern CView gView;

#endif // VIEW_H