#ifndef PART_SHAPE_H
#define PART_SHAPE_H

#include <refcount.h>

#include "ResourceMaterial.h"
#include "Part.h"

class PShape : public CPart {
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
    float Thickness;
private:
    char Pad1[0xc];
public:
    float MassDepth;
    CRawVector<v2, CAllocatorMMAligned128> Polygon;
    CRawVector<unsigned int> Loops;
    CP<RMaterial> MMaterial;
    CP<RMaterial> OldMMaterial;
};

#endif // PART_SHAPE_H