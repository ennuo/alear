#include "Resource.h"
#include "ResourceSystem.h"

#include <hook.h>

void CResource::BlockUntilLoaded()
{
    CResource* resource = this;
    BlockUntilResourcesLoaded(&resource, 1);
}

MH_DefineFunc(CResource_Duplicate, 0x000a1754, TOC0, ReflectReturn, CResource*, CResource*);
ReflectReturn CResource::Duplicate(CResource* resource)
{
    return CResource_Duplicate(this, resource);
}