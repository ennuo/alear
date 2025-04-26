#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <refcount.h>
#include <vector.h>

#include "frustcull.h"
#include "ResourceGFXTexture.h"
#include "GuidHash.h"


class CThing;
class CMesh;
class CMorphWeightInstance;

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

u32 AllocateHugeBones(u32 numBonesRequired);

class __attribute__((aligned(0x80))) CMeshInstance {
public:
    CMeshInstance();
    void SetMesh(CMesh* mesh);
    void InitInstanceForRender(u32 first_bone_index, u32 num_bones, CVector<void*>* decals, CVector<void*>* eyetoy);
    void AddToBuckets(bool cast_shadows, bool, bool);
    void Invalidate();
public:
    CCullBone CullBone;
    v4 InstanceColor;
    v4 TintColor;
    v4 NudgePosRad;
    v4 NudgeDirStrength;
    v4 ExternalForce;
    v2 UVScale;
    v2 UVOffset;
    CMesh* Mesh;
    CP<RTexture> InstanceTexture;
    u32 InstanceTextureRemap;
    u32 InstanceTextureApplyIdx;
    CVector<void*>* InstancePrimitives;
    CMorphWeightInstance* InstanceMorphWeights;
    u32 SPUOutputOffset_Pos;
    u32 SPUOutputOffset_Nor;
    u32 SPUOutputOffset_PosOld;
    u32 SPUOutputOffset_PosOldOld;
    v4* PosAddr;
    v4* PosOldAddr;
    v4* PosOldOldAddr;
    v4* NorAddr;
    v4* NorOldCopy;
    m44* ClusterMatrix;
    m44* OldClusterMatrix;
    m44* OldOldClusterMatrix;
    u32 OldCollisionEllipse;
    u32 FramesAlive;
    u32 LastFrameSimulated;
    u32 VisibilitySoFar;
    // u32 NewVisibilityBits;
    u32 LastVisibleFrameNumber;
    u32 FirstBoneIndex;
    u32 ICacheX;
    u32 ICacheY;
    u32 ICacheW;
    u32 ICacheH;
    u32 ICacheWSMiss;
    CMeshInstance* ICachePrev;
    CMeshInstance* ICacheNext;
    CVector<void*>* Decals;
    CVector<void*>* EyetoyData;
    unsigned int IsGeneratedMesh : 1;
    unsigned int RenderToTexture : 1;
    unsigned int DisableSoftbody : 1;
    unsigned int DisableCollisionAdd : 1;
    unsigned int Dissolving : 1;
    unsigned int IsDecoration : 1;
    unsigned int AlreadyRenderedDecals : 1;
    unsigned int Splatting : 1;
    CThing* MyThing; // 376
    float* BlendClusterRigidity;
    CMeshInstance* Exclude;
    void* InstanceSoftPhys;
    u8* InstanceClusterData;
    CRawVector<u16*> RepeatIndices;
    u16 ActiveRepeats;
    u16 Repeats;
    void* MeshticleState;
};

struct InstanceList {
    u32 Size;
    u32* Key;
    CMeshInstance** MeshInstance;
};

extern InstanceList gFilteredBucketForRaycast;
extern InstanceList gRenderBucket;

#endif // SCENE_GRAPH_H