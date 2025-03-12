#ifndef PART_SHAPE_H
#define PART_SHAPE_H

#include <refcount.h>

#include "ResourceMaterial.h"
#include "GameEnums.inl"
#include "Part.h"

class PShape : public CPart {
struct Forked {
    v2 Min;
    v2 Max;
    CRawVector<v2, CAllocatorMMAligned128> GlobalPolygon; // 0x20
    CVector<void*> Convexes; // technically CVector<CCompactConvex, CAllocatorMMAligned128>, but need a dummy type
    v2* SharedVertices;
    // float BrightnessHack;
};
public:
    // inline float& GetBrightness() { return Game.BrightnessHack; }
    // inline float& GetBrightnessOff() { return Rend.BrightnessHack; }
    // inline bool HasCustomData()
    // {
    //     return false;
    //     // return GetBrightness() != 1.0f || GetBrightness() != 0.0f;
    // }

    void SetCollidableGame(bool collidable);
    void SetCollidablePoppet(bool collidable);
    void SetMaterial(RMaterial* material);
    void InitialisePolygon();
    void SetPolygon(CRawVector<v2, CAllocatorMMAligned128> const& vertices, CRawVector<unsigned int> const& indices);
public:
    float Moment;
    v4 EditorColourTint;
    v4 EditorColour;
    m44 COM;
    ELethalType LethalType;
    float BevelSize;
    u32 SoundEnumOverride;
    bool CollidableGame;
    bool CollidablePoppet;
    bool CollidableWithParent;
    u8 InteractPlayMode;
    u8 InteractEditMode;
private:
    char Pad0[0xf];
public:
    floatInV2 Thickness;
    float MassDepth;
    CRawVector<v2, CAllocatorMMAligned128> Polygon;
    CRawVector<unsigned int> Loops;
    CP<RMaterial> MMaterial;
    CP<RMaterial> OldMMaterial;
private:
    char Pad2[0x2c];
public:
    // theres padding here, so i can just sneak it in
    Forked Game; // 0xf0
    Forked Rend;
    Forked* Fork;
    CRawVector<unsigned int, CAllocatorMMAligned128> SharedIndices; // 0x174
    s16 ElectricFrame;
    s16 FireFrame;
    s16 GasFrame;
};

#endif // PART_SHAPE_H