#include <thing.h>
#include <ResourceGFXMesh.h>
#include "PartMaterialOverride.h"

#include <cell/DebugLog.h>
#include <cell/thread.h>

#include <GFXApi.h>

// CVector<CPrimitive>& GetPrimitivesForRendering(CMeshInstance* instance)
// {
//     const CThing* thing = instance->MyThing;
//     if (thing == NULL) return instance->Mesh->Primitives;
    
//     PMaterialOverride* part_override = thing->GetPMaterialOverride();
//     if (part_override == NULL || !part_override->Mesh || !part_override->Mesh->IsLoaded()) return instance->Mesh->Primitives;

//     // Make sure we're applying the overrides to the proper mesh.
//     if (instance->Mesh != &part_override->Mesh->mesh) return instance->Mesh->Primitives;

//     // No point in applying our instance primitives if there are no overrides.
//     if (part_override->Overrides.size() == 0) return instance->Mesh->Primitives;

//     part_override->Refresh();
//     return part_override->Primitives;
// }


void OnPreBindDrawCallPrimitives(CMeshInstance* instance)
{
    const CThing* thing = instance->MyThing;
    if (thing == NULL) return;

    PMaterialOverride* part_override = thing->GetPMaterialOverride();
    if (part_override == NULL || !part_override->Mesh || !part_override->Mesh->IsLoaded()) return;

    // Make sure we're applying the overrides to the proper mesh.
    if (instance->Mesh != &part_override->Mesh->mesh) return;

    // No point in applying our instance primitives if there are no overrides.
    if (part_override->Overrides.size() == 0) return;

    // It isn't that computationally expensive to just refresh the primitives
    // every time honestly, although this should realistically be done in a pre-setup step.
    part_override->Refresh();
    part_override->Apply(instance);
}

PMaterialOverride::~PMaterialOverride()
{
    PRenderMesh* part_render_mesh = GetThing()->GetPRenderMesh();
    if (part_render_mesh == NULL) return;
    CMeshInstance* instance = part_render_mesh->MeshInstance;;
    if (instance == NULL) return;
    instance->InstancePrimitives = NULL;
}

void PMaterialOverride::Refresh()
{
    // Just doing it this way to avoid re-allocations, 
    // probably should've just had operator= resize the vector
    // instead of resetting it, but whatever.
    Primitives.try_resize(0);
    Primitives.try_reserve(Mesh->mesh.Primitives.size());

    for (int i = 0; i < Mesh->mesh.Primitives.size(); ++i)
    {
        const CPrimitive& primitive = Mesh->mesh.Primitives[i];
        Primitives.push_back(primitive);
    }

    // Apply all our override materials to affected regions.
    for (CRegionOverride* region = Overrides.begin(); region != Overrides.end(); ++region)
    for (CPrimitive* primitive = Primitives.begin(); primitive != Primitives.end(); ++primitive)
    {
        if (primitive->Region != region->Region) continue;
        primitive->Material = region->Material;
    }
}

void PMaterialOverride::Apply(CMeshInstance* instance)
{
    instance->InstancePrimitives = &Primitives;

    CMesh& mesh = Mesh->mesh;
    u32 num_verts = mesh.NumVerts;
    u32 num_attr = mesh.AttributeCount;
    u32 data_size = num_verts * num_attr * 0x8;

    
    void* handle = gSMemPools[NGfx::GetCurDynamicNormalPool()].Alloc(data_size, 0x80, 0x5dc00);
    memcpy(handle, mesh.AttributeData.GetCachedAddress(), data_size);

    // The vertex data is unaligned with any mesh that has UVs, so can't use vectors?
    // It's probably slow to do this on the PPU anyway versus the SPU, but I don't feel like doing all that
    // and this is probably only run for like 1 or 2 meshes per frame in the most outlier of circumstances.
    float* attr = (float*)handle;
    for (int i = 0; i < num_verts; ++i)
    {
        


        attr += num_attr * 2;
    }





}