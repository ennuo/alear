#include "resources/ResourceGuidList.h"
#include <Serialise.h>
#include <Variable.h>

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;


template <typename R>
ReflectReturn Reflect(R& r, RGuidList& d)
{
    ReflectReturn ret;
    ADD(List);
    return ret;
}

template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, RGuidList& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, RGuidList& d);

#undef ADD