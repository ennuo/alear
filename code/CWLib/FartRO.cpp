#include "FartRO.h"
#include <hook.h>

MH_DefineFunc(MakeROCache, 0x000869ac, TOC0, CFartRO*, CFilePath& farc_file, bool enabled, bool secure);

MH_DefineFunc(CFartRO_CloseCache, 0x00085790, TOC0, void, CFartRO*, bool);
void CFartRO::CloseCache(bool updating)
{
    CFartRO_CloseCache(this, updating);
}

MH_DefineFunc(CFartRO_OpenCache, 0x00086680, TOC0, bool, CFartRO*);
bool CFartRO::OpenCache()
{
    return CFartRO_OpenCache(this);
}