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


#endif // SOFTBODY_H