#include "mmalex.h"



namespace mmalex {
    MH_DefineFunc(__stod, 0x0059c400, TOC1, double, const char*, char**, long);
    MH_DefineFunc(__stoll, 0x0059ce94, TOC1, long long, const char *, char **, int);
    MH_DefineFunc(__FSin, 0x004722d0, TOC1, float, float, unsigned int);
    MH_DefineFunc(_sqrtf, 0x004719d0, TOC1, float, float);
    MH_DefineFunc(crc32, 0x005305e8, TOC1, u32, u32, const char*, u32);
};

extern "C"
{
    MH_DefineFunc(_sqrtf, 0x004719d0, TOC1, float, float);
    float ps3_sqrtf(float f) { return _sqrtf(f); }
}