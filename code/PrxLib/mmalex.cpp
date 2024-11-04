#include "mmalex.h"

#include <hook.h>

namespace mmalex {
    MH_DefineFunc(__stod, 0x0059c400, TOC1, double, const char*, char**, long);
    MH_DefineFunc(__stoll, 0x0059ce94, TOC1, long long, const char *, char **, int);
    MH_DefineFunc(__FSin, 0x004722d0, TOC1, float, float, unsigned int);
    MH_DefineFunc(_sqrtf, 0x004719d0, TOC1, float, float);
};

extern "C"
{
    MH_DefineFunc(_sqrtf, 0x004719d0, TOC1, float, float);
    float ps3_sqrtf(float f) { return _sqrtf(f); }
}