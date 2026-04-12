#pragma once

#include <FartRO.h>

class CMutableCache : public CFartRO {
public:
    bool Put(CHash&, const void*, u32);
};
