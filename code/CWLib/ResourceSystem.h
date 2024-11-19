#ifndef RESOURCE_SYSTEM_H
#define RESOURCE_SYSTEM_H


#include "Resource.h"

#include <vector.h>
#include <CritSec.h>
#include <ResourcePointer.h>

extern bool gTestSuiteResourcesDisabled;

CP<CResource> LoadResource(CResourceDescriptorBase const& desc, CStreamPriority stream_priority_override, u32 flags, bool can_create);
void LoadResource(CResource* rv, CStreamPriority stream_priority_override);
CResource* AllocateNewResource(EResourceType type, u32 flags);
extern CP<CResource> (*LoadResourceByFilenameGeneric)(CFilePath const& filename, EResourceType type, unsigned int flags, CStreamPriority stream_priority_override, bool can_create);

template<typename T>
T* AllocateNewResource(u32 flags)
{
    return (T*)AllocateNewResource(GetResourceType<T>(), flags);
}

template<typename T>
CP<T> LoadResourceByKey(int key, u32 flags, CStreamPriority stream_priority_override)
{
    if (gTestSuiteResourcesDisabled) return NULL;
    CResourceDescriptorBase desc(GetResourceType<T>(), key);
    CP<CResource> resource = LoadResource(desc, stream_priority_override, flags, false);
    return CP<T>((T*)resource.GetRef());
}

template<typename T>
CP<T> LoadResource(CResourceDescriptor<T> const& desc, u32 flags, CStreamPriority stream_priority_override, bool can_create)
{
    CP<CResource> resource = LoadResource(desc, stream_priority_override, flags, false);
    return CP<T>((T*)resource.GetRef());
}

template<typename T>
CP<T> LoadResourceByFilename(CFilePath const& filename, unsigned int flags, CStreamPriority stream_priority_override, bool can_create)
{
    CP<CResource> resource = LoadResourceByFilenameGeneric(filename, GetResourceType<T>(), flags, stream_priority_override, can_create);
    return CP<T>((T*)resource.GetRef());
}

extern bool (*BlockUntilLoaded_)(EWaitForStreamingResources streaming_resources, CResource** wait_for_resources, unsigned int count);
extern bool (*BlockUntilResourcesLoaded)(CResource** resources, u32 count);
bool _BlockUntilAllResourcesLoaded(EWaitForStreamingResources streaming_resources);


extern CResource* (*FindResourceInList)(CResourceDescriptorBase const& desc);
extern void (*GetResourceSet)(CRawVector<CResource*>& out, EResourceType resource_type);
extern void (*UnloadResource)(CP<CResource> resource);

typedef CRawVector<CResource*> CWeakResourceArray;
extern CWeakResourceArray gResourceArray;
extern CCriticalSec gResourceCS;

#endif // RESOURCE_SYSTEM_H