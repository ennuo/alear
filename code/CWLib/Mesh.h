#ifndef MESH_H
#define MESH_H

#include <vector.h>
#include <refcount.h>

#include "GfxPool.h"
#include "ResourceGfxMaterial.h"

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
private:
    char _Pad0[60];
public:
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