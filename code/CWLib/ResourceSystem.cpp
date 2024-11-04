#include "ResourceSystem.h"
#include "hook.h"

MH_DefineFunc(BlockUntilLoaded_, 0x0008d154, TOC0, bool, EWaitForStreamingResources streaming_resources, CResource** wait_for_resources, unsigned int count);
MH_DefineFunc(BlockUntilResourcesLoaded, 0x0008f174, TOC0, bool, CResource** resources, u32 count);
MH_DefineFunc(FindResourceInList, 0x0008a3f0, TOC0, CResource*, CResourceDescriptorBase const& desc);
MH_DefineFunc(UnloadResource, 0x0008a7c0, TOC0, void, CP<CResource> resource);
MH_DefineFunc(GetResourceSet, 0x000899d4, TOC0, void, CRawVector<CResource*>& out, EResourceType resource_type);

MH_DefineFunc(LoadResource_Descriptor, 0x0008e194, TOC0, CP<CResource>, CResourceDescriptorBase const&, CStreamPriority, u32, bool);
MH_DefineFunc(LoadResource_Internal, 0x0008d6a0, TOC0, void, CResource*, CStreamPriority);

MH_DefineFunc(AllocateNewResource_Internal, 0x00088a98, TOC0, CResource*, EResourceType, u32);
CResource* AllocateNewResource(EResourceType type, u32 flags)
{
    return AllocateNewResource_Internal(type, flags);
}

void LoadResource(CResource* rv, CStreamPriority stream_priority_override)
{
    LoadResource_Internal(rv, stream_priority_override);
}

CP<CResource> LoadResource(CResourceDescriptorBase const& desc, CStreamPriority stream_priority_override, u32 flags, bool can_create)
{
    return LoadResource_Descriptor(desc, stream_priority_override, flags, can_create);
}

bool _BlockUntilAllResourcesLoaded(EWaitForStreamingResources streaming_resources)
{
    return BlockUntilLoaded_(streaming_resources, NULL, 0);
}

const char* gResourceNames[RTYPE_LAST];