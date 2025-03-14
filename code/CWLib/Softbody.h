#ifndef SOFTBODY_H
#define SOFTBODY_H

struct ClusterName {
    char Name[32];
};

class SoftbodyClusterData {
public:
    u32 ClusterCount;
    v4* RestCenterOfMass;
    m44* RestDyadicSum;
    float* RestQuadraticDyadicSum;
    ClusterName* Name;
};

class CImplicitEllipsoid {
public:
    m44 Transform;
    s32 ParentBone;
    s32 AffectWorldOnly;
};


#endif // SOFTBODY_H