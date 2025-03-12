#ifndef MESH_H
#define MESH_H

#include <vector.h>
#include <refcount.h>

#include <spring.h>


#include "Softbody.h"
#include "GfxPool.h"
#include "ResourceGfxMaterial.h"
#include "SceneGraph.h"

class CMeshShapeVertex {
public:
    v4 LocalPos;
    v4 LocalNormal;
    u32 BoneIndex;
};

class CMeshShapeInfo {
public:
    u32 NumVerts;
    u32 IsPointCloud;
};

class CAnimBone {
public:
    u32 AnimHash;
    int Parent;
    int FirstChild;
    int NextSibling;
};

class CBone : public CAnimBone {
public:
    m44 SkinPoseMatrix;
    m44 InvSkinPoseMatrix;
    v4 OBBMin;
    v4 OBBMax;
    v4 BoundBoxMin;
    v4 BoundBoxMax;
    v4 BoundSphere;
    char Name[32];
    u32 Flags;
    CRawVector<CMeshShapeInfo> ShapeInfos;
    CRawVector<CMeshShapeVertex> ShapeVerts;
    float ShapeMinZ;
    float ShapeMaxZ;
};

class CPrimitive {
public:
    CP<RGfxMaterial> Material;
    void* TextureAlternatives; // CP<RTextureList> never used dont care
    u32 MinVert;
    u32 MaxVert;
    u32 FirstIndex;
    u32 NumIndices;
    u32 Region;
    u32 PAD0;
};

class CMesh {
public:
    v4 SoftbodyContainingBoundBoxMin;
    v4 SoftbodyContainingBoundBoxMax;
    u32 NumVerts;
    u32 NumIndices;
    u32 NumEdgeIndices;
    u32 NumTris;
    u32 StreamCount;
    u32 AttributeCount;
    u32 SourceStreamOffsets[35];
    u32 MorphCount;
    u32 BevelVertexCount;
    CVector<CBone> Bones;
    CRawVector<CCullBone> CullBones;
    SoftbodyClusterData SoftbodyCluster;
    SoftbodySpringVec SoftbodySprings;
    bool ImplicitBevelSprings;
    u8 PrimitiveType;
    CGfxHandle Indices;
    CRawVector<unsigned int> BevelStitchingPairs;
    CVector<CPrimitive> Primitives;
    CGfxHandle SourceGeometry;
    CGfxHandle AttributeData;
    char MorphNames[32][16];
    float MinUV[2];
    float MaxUV[2];
    float AreaScaleFactor;
};

#endif // MESH_H