#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <refcount.h>

#include "frustcull.h"
#include "ResourceGFXTexture.h"
#include "GuidHash.h"


class CThing;

class CDecal {
public:
    CP<RTexture> texture;
    CGUID PlanGUID;
    float u, v;
    float xvecu, xvecv, yvecu, yvecv;
    u32 PlayModeFrame;
    u16 meta_data_idx;
    u16 num_meta_data;
    u16 placed_by;
private:
    u16 packed_col;
    u8 type;
    bool ScorchMark;
    bool IsDead;
};

class __attribute__((aligned(0x80))) CMeshInstance {
public:
    CCullBone CullBone;
    v4 InstanceColor;
private:
    char Pad[0xe8];
public:
    CThing* MyThing;
private:
    char Pad2[0x84];
};

struct InstanceList {
    u32 Size;
    u32* Key;
    CMeshInstance** MeshInstance;
};

extern InstanceList gFilteredBucketForRaycast;
extern InstanceList gRenderBucket;

#endif // SCENE_GRAPH_H