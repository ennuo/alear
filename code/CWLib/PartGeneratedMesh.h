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
    void InitializeExtraData();
    inline bool HasCustomData()
    {
        return TextureAnimationSpeed != 1.0f || TextureAnimationSpeedOff != 0.0f;
    }
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
private:
    char Pad[89];
public:
    float TextureAnimationSpeed;
    float TextureAnimationSpeedOff;
};

#endif // PART_GENERATED_MESH_H