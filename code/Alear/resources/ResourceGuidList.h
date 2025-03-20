#ifndef RESOURCE_GUID_LIST_H
#define RESOURCE_GUID_LIST_H

#include <refcount.h>
#include <vector.h>
#include <GuidHash.h>

#include <Resource.h>
#include <ResourceDescriptor.h>

class RGuidList : public CResource {
public:
    inline RGuidList(EResourceFlag flags) : CResource(flags, RTYPE_GUID_LIST), List() {}
public:
    u32 GetSizeInMemory() { return sizeof(RGuidList); }
public:
    CVector<CGUID> List;
};


#include "Serialise.h"
#include "Variable.h"
template <typename R>
ReflectReturn Reflect(R& r, RGuidList& d);

#endif // RESOURCE_GUID_LIST_H