#ifndef RESOURCE_SYSTEM_H
#define RESOURCE_SYSTEM_H

#include "Resource.h"
extern bool g_TestSuiteResourcesDisabled;

extern CP<CResource> (*LoadResource)(CResourceDescriptorBase const&, CStreamPriority, u32, bool);

template<typename T>
CP<T> LoadResourceByKey(int key, u32 flags, CStreamPriority stream_priority_override)
{
    if (g_TestSuiteResourcesDisabled) return NULL;
    CResourceDescriptorBase desc(GetResourceType<T>(), key);
    CP<CResource> resource = LoadResource(desc, stream_priority_override, flags, false);
    return CP<T>((T*)resource.GetRef());
}

extern bool (*BlockUntilResourcesLoaded)(CResource** resources, u32 count);
#define BlockUntilResourceLoaded(resource) BlockUntilResourcesLoaded((CResource**)&resource, DONT_WAIT_FOR_STREAMING_RESOURCES)

#endif // RESOURCE_SYSTEM_H