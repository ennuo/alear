#ifndef STICKER_INFO_H
#define STICKER_INFO_H

#include <vector.h>
#include <refcount.h>
#include <Colour.h>

#include "ResourceDescriptor.h"
#include "ColorCorrection.h"

class CPaintControlPointUnsquished {
public:
    float x;
    float y;
    float start_radius;
    float end_radius;
};

class CStickerInfo {
public:
    v4 Up;
    v4 Across;
    CRawVector<v2,CAllocatorMMAligned128> Outline;
    CP<RTexture> Texture;
    float Height;
    float Width;
    float Angle;
    float Scale;
    bool Reversed;
    bool Placed;
    v4 Offset;
    bool Stamping;
    v4 TintColor;
    CResourceDescriptor<RPlan> RefPlan;
    CRawVector<CPaintControlPointUnsquished> StrokeUncompressed;
    CPaintControlPointUnsquished LastStrokePos;
    bool IsStroking;
    float StrokeRadius;
    v2 StrokeDirection;
    float LastStrokeScale;
    c32 PaintColour;
    CEyetoyData EyetoyData;
    CP<RTexture> EyetoyBackup;
};

#endif // STICKER_INFO_H