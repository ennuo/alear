#ifndef GOOEY_RENDER_H
#define GOOEY_RENDER_H

#include <refcount.h>
#include <Colour.h>


#include "network/NetworkUtilsNP.h"
#include "ResourceGFXTexture.h"
#include "ResourceGFXFont.h"

class CSDFIconParams {
public:
    CP<RTexture> texture;
    float u_1;
    float v_1;
    float u_2;
    float v_2;
    int type;
    u32 SDF;
    c32 col;
    NetworkOnlineID OnlineID;
};

namespace NHUD {
    extern void (*PreloadGlyphs)(RFontFace* font, tchar_t* current, tchar_t* end);
}


#endif // GOOEY_RENDER_H