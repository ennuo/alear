#ifndef DEBUG_CAMERA_H
#define DEBUG_CAMERA_H

#include "Camera.h"

class CDebugCamera {
public:
    void Update(bool maverick);
    void Apply(CCamera* camera);
public:
    float Z;
    float TargetZ;
    float Yaw;
    float Pitch;
    v4 TargetFoc;
    v2 MavFocOffset;
    CDebugCamera* Preset;
    v4 Pos;
    v4 Foc;
};

#endif // DEBUG_CAMERA_H