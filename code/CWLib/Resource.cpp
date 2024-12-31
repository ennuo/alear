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

MH_DefineFunc(CResource_Unload, 0x000bd90c, TOC0, void, CResource*);
void CResource::Unload()
{
    CResource_Unload(this);
}

ReflectReturn CResource::LoadFinished(SRevision const& load_revision)
{
    return REFLECT_OK;
}

void CResource::AddDependencies(CVector<CP<CResource> >& dependencies)
{
    return;
}

u32 CResource::GetSizeInMemory()
{
    return sizeof(CResource);
}

u32 CResource::GetSizeInGfxMemoryPool(u32 location)
{
    return 0;
}

MH_DefineFunc(CResource_dtor, 0x000be71c, TOC0, void, CResource*);
CResource::~CResource()
{
    CResource_dtor(this);
}

MH_DefineFunc(CResource_ctor, 0x000ae094, TOC0, void, CResource*, EResourceFlag, EResourceType);
CResource::CResource(EResourceFlag flags, EResourceType type)
{
    CResource_ctor(this, flags, type);
}