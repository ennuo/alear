#ifndef CAMERA_H
#define CAMERA_H

class CCamera {
public:
    float OrthoLength;
    float FOV;
    float Aspect;
    float ZNear;
    float ZFar;
    m44 ViewProjMatrix;
    m44 ViewMatrix;
    m44 CameraMatrix;
    m44 ProjMatrix;
    m44 InvViewProjMatrix;
    m44 OldViewProjMatrix;
};

#endif // CAMERA_H