#include "gooey/GooeyRender.h"
#include <hook.h>

namespace NHUD
{
    MH_DefineFunc(PreloadGlyphs, 0x00305f38, TOC0, void, RFontFace* font, tchar_t* current, tchar_t* end);
}