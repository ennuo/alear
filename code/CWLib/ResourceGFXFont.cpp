#include "ResourceGFXFont.h"


MH_DefineFunc(StartDrawText, 0x00099b00, TOC0, void, bool faded, CCamera* cam, m44* oldprojmat, bool scribble, bool allow_clipping, float screen_x, float screen_y);
MH_DefineFunc(EndDrawText, 0x00099a4c, TOC0, void);
MH_DefineFunc(DrawText, 0x00099bc0, TOC0, void, RFontFace* face, tchar_t const* str, float x, float y, float z, float size, u32 color, float linespacing);