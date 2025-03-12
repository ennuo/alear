#ifndef RESOURCE_SETTINGS_SOFT_PHYS_H
#define RESOURCE_SETTINGS_SOFT_PHYS_H

#include "Resource.h"
#include "Softbody.h"

class RSettingsSoftPhys : public CResource {
public:
    float* ClusterRigidity;
    float* ClusterEffect;
    int* ClusterParamSet;
    v2 Damping;
    v2 CollideDamping;
    v2 Force;
    v2 Wind;
    float WindStrength;
    v2 WindScale;
    float WindFrequency;
    float BoundingBoxMultiplier;
    int SpringIterations;
    int SpringConvexIterations;
    float SpringScale;
    float ConvexCollisionOffset;
    float MyConvexCollisionOffset;
    float OtherConvexCollisionOffset;
    float CollideStickyness;
    float CollideThresh;
    float CollideEps0;
    float CollideEps1;
    float CollideEps2;
    float CollideEps3;
    float SquishLimit;
    bool UseQuadraticSquishyness;
    bool EllipseCollision;
    bool InsideEllipseCollision;
    bool ConvexCollision;
    bool PlaneCollision;
    bool ClusterEllipseCollision;
    u32 ClusterCount;
    v2 ExtraDamping[3];
    v2 ExtraCollideDamping[3];
    v2 ExtraForce[3];
    v2 ExtraWind[3];
    float ExtraWindStrength[3];
    v2 ExtraWindScale[3];
    float ExtraWindFrequency[3];
    SoftbodyClusterData* MySoftbodyClusterData;
};

#endif // RESOURCE_SETTINGS_SOFT_PHYS_H