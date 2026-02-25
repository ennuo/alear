#include <ResourceDescriptor.h>
#include <GuidHashMap.h>
#include <SerialisedResource.h>

CSerialisedResource::CSerialisedResource(const CResourceDescriptorBase& desc) :
CBaseCounted(), Descriptor(desc), Data()
{

}
