#include "Fart.h"
#include "FartRO.h"

#include <hook.h>

MH_DefineFunc(InitCaches, 0x000814f0, TOC0, bool);
MH_DefineFunc(CloseCaches, 0x0007f2f0, TOC0, bool);

MH_DefineFunc(IncorporateFartRO, 0x0008026c, TOC0, bool, CFartRO* cache);
