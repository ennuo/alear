#pragma once

#include <ResourceDescriptor.h>
#include <SerialiseEnums.h>

// template <typename R>
// ReflectReturn ReflectDescriptor(R& r, CResourceDescriptorBase& d, bool cp, bool type) // 23
// {
//     return REFLECT_NOT_IMPLEMENTED;
// }



template <typename R, typename D>
ReflectReturn ReflectEnumAsU8(R& r, D* d) // 196
{
    u8 b = d;
    return Reflect<R>(r, d);
}
