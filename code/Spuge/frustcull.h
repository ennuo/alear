#ifndef FRUST_CULL_H
#define FRUST_CULL_H

#include "mmtypes.h"

class CCullBone {
public:
    m44 InvSkinPoseMatrix;
    v4 BoundBoxMin;
    v4 BoundBoxMax;
    v4 AABBMin;
    v4 AABBMax;
};

#endif // FRUST_CULL_H