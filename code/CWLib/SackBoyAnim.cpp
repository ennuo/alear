#include "SackBoyAnim.h"
#include <hook.h>

MH_DefineFunc(CSackBoyAnim_LoadAnims, 0x000f975c, TOC0, void, CSackBoyAnim*, CAnimBank*, bool);
void CSackBoyAnim::LoadAnims(CAnimBank* ab, bool cached)
{
    CSackBoyAnim_LoadAnims(this, ab, cached);
}