#ifndef RESOURCE_GFX_FONT_H
#define RESOURCE_GFX_FONT_H

#include "refcount.h"
#include "Resource.h"
#include "Camera.h"
#include "FontEnum.h"

class RFontFace : public CResource {
public:

};

extern void (*StartDrawText)(bool faded, CCamera* cam, m44* oldprojmat, bool scribble, bool allow_clipping, float screen_x, float screen_y);
extern void (*EndDrawText)();
extern void (*DrawText)(RFontFace* face, tchar_t const* str, float x, float y, float z, float size, u32 color, float linespacing);

extern CP<RFontFace> gFont[FONT_LAST];

#endif // RESOURCE_GFX_FONT_H