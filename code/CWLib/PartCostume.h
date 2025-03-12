#ifndef PART_COSTUME_H
#define PART_COSTUME_H

#include "Part.h"
#include "Resource.h"
#include "ResourceGFXMesh.h"
#include "ResourceGfxMaterial.h"
#include "ResourceDescriptor.h"
#include "MorphConstants.h"
#include "SceneGraph.h"

class RPlan;


enum ECostumePieceCategory 
{
    COSTUMEPART_FACE_BEARD=0,
    COSTUMEPART_FACE_FEET=1,
    COSTUMEPART_FACE_EYES=2,
    COSTUMEPART_FACE_GLASSES=3,
    COSTUMEPART_FACE_MOUTH=4,
    COSTUMEPART_FACE_MUSTACHE=5,
    COSTUMEPART_FACE_NOSE=6,
    COSTUMEPART_HAIR=7,
    COSTUMEPART_HEAD=8,
    COSTUMEPART_NECK=9,
    COSTUMEPART_TORSO=10,
    COSTUMEPART_LEGS=11,
    COSTUMEPART_HANDS=12,
    COSTUMEPART_WAIST=13,
    COSTUMEPART_TEMP_START=14,
    COSTUMEPART_COUNT=15
};



class CCostumePiece {
public:
    CMeshInstance MeshInstance;
    CP<RMesh> Mesh;
    CResourceDescriptor<RPlan> OriginalPlan;
    u32 CategoriesUsed;
    u32 MorphParamRemap[32];
    CMorphWeightInstance MorphWeightInstance;
    CVector<CPrimitive> MeshPrimitives; // 0x330
private:
    char Pad[0x44];
};

class PCostume : public CPart {
public:
    void SetPowerup(RMesh* mesh, CResourceDescriptor<RPlan>& original_plan);
    void RemovePowerup(CResourceDescriptor<RPlan>& original_plan);
    void SetMaterial(RGfxMaterial const* mat, CResourceDescriptor<RPlan> const& plan);
public:
    inline CP<RGfxMaterial> GetCurrentMaterial() const { return Mat; }
    inline const CResourceDescriptor<RPlan>& GetCurrentMaterialPlan() const { return MatPlan; }
public:
    bool NeedCostumeFixup; // 0xc
    CP<RMesh> Mesh; // 0x10
    CP<RGfxMaterial> Mat; // 0x14
    CResourceDescriptor<RPlan> MatPlan; //  0x18
    CRawVector<unsigned int> MeshPartsHidden; // 0x40
    CVector<CPrimitive> MeshPrimitives; // 0x4c
    int CostumeChangeAddedParts;
    int CostumeChangeRemovedParts;
    bool CostumeChange;
    bool CostumeChangeMaterial;
    u32 RequiresDecalRender;
    CCostumePiece CostumePieceVec[COSTUMEPART_COUNT]; // 0x80
};

#endif // PART_COSTUME_H