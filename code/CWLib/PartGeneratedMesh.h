#ifndef PART_GENERATED_MESH_H
#define PART_GENERATED_MESH_H

#include <refcount.h>
#include <GuidHash.h>

#include "Part.h"
#include "SceneGraph.h"
#include "ResourceGfxMaterial.h"
#include "ResourceBevel.h"
#include "Mesh.h"

class PGeneratedMesh : public CPart {
public:
    u32 BaseMeshVertCount;
    v4 BoundingBoxMin;
    v4 BoundingBoxMax;
    v4 UVOffset;
    m44 StashedMatrix;
    CMeshInstance* MeshInstance;
    CP<RGfxMaterial> GfxMaterial;
    CP<RBevel> Bevel;
    typedef const CMesh* meshtype;
    meshtype SharedMesh;
    u32 SharedMeshDirty;
    u32 LastVisibleFrameNumber_IncludingShadows;
    CGUID PlanGUID;
    bool AsyncJobWaiting;
    bool Visible;
    bool WasJustVisible;
};

#endif // PART_GENERATED_MESH_H