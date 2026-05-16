#pragma once

#include <FartRO.h>

class CMutableCache : public CFartRO {
public:
    CMutableCache(const CFilePath& _fp);
public:
    bool Put(CHash&, const void*, u32);
};
