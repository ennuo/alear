#include "Fart.h"
#include "FartRO.h"

#include <hook.h>

MH_DefineFunc(InitCaches, 0x000814f0, TOC0, bool);
MH_DefineFunc(CloseCaches, 0x0007f2f0, TOC0, bool);

MH_DefineFunc(IncorporateFartRO, 0x0008026c, TOC0, bool, CFartRO* cache);

MH_DefineFunc(GetFileDataFromCaches, 0x000816bc, TOC0, bool, CHash&, ByteArray&);
MH_DefineFunc(SaveFileDataToCache, 0x0007f458, TOC0, bool, CacheType type, const void* data, u32 size, CHash& out);

bool GetResourceReader(CHash& hash, SResourceReader& out)
{
    for (int i = 0; i < CT_COUNT; ++i)
    {
        CCache* cache = gCaches[i];
        if (cache == NULL) continue;
        if (cache->GetReader(hash, out)) return true;
    }

    return false;
}