#include "View.h"
#include <hook.h>

MH_DefineFunc(GGetLevelSettings, 0x001fc9d4, TOC0, PLevelSettings*, PWorld* world);
