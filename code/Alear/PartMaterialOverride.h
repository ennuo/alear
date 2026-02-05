#ifndef PART_MATERIAL_OVERRIDE_H
#define PART_MATERIAL_OVERRIDE_H

#include <refcount.h>
#include <vector.h>
#include <MMString.h>

#include <Part.h>
#include <ResourceDescriptor.h>

class CMeshInstance;
class CPrimitive;

class CRegionOverride {
public:
    inline CRegionOverride() : Region(), MaterialPlan(), Material(), UVScale(), Color(), Brightness() {}
public:
    u32 Region;
    CResourceDescriptor<RPlan> MaterialPlan;
    CP<RGfxMaterial> Material;
    v4 UVScale;
    u32 Color;
    u8 Brightness;
};

class PMaterialOverride : public CPart {
public:
    inline PMaterialOverride() : CPart(), Primitives(), Overrides(), Mesh(), Color(), Brightness() {}
    ~PMaterialOverride();
public:
    void Refresh();
    void Apply(CMeshInstance* instance);
public:
    CVector<CPrimitive> Primitives;
public:
    CVector<CRegionOverride> Overrides;
    CP<RMesh> Mesh;
    u32 Color;
    u8 Brightness;
};

#endif // PART_MATERIAL_OVERRIDE_H