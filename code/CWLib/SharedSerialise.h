#ifndef SHARED_SERIALISE_H
#define SHARED_SERIALISE_H

#include "SerialiseEnums.h"

template <typename R, typename D>
ReflectReturn ReflectEnumAsU8(R& r, D* d)
{
    u8 b = d;
    return Reflect<R>(r, d);
}

#endif // SHARED_SERIALISE_H