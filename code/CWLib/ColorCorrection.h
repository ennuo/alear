#ifndef COLOR_CORRECTION_H
#define COLOR_CORRECTION_H

#include <refcount.h>
#include <ReflectionVisitable.h>

class RTexture;

class CColorCorrection {
public:
    float Saturation;
    float HueShift;
    float Brightness;
    float Contrast;
    float TintHue;
    float TintAmount;
};

class CEyetoyData : public CReflectionVisitable {
public:
    CP<RTexture> Frame;
    CP<RTexture> AlphaMask;
    CColorCorrection ColorCorrectionSrc;
    m44 ColorCorrection;
    u32 PreloadID;
};

#endif // COLOR_CORRECTION_H