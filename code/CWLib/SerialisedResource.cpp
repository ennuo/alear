#include <ResourceDescriptor.h>
#include <GuidHashMap.h>
#include <SerialisedResource.h>

CSerialisedResource::CSerialisedResource() : CBaseCounted(), Descriptor(), LoosePath(), Data()
{

}

CSerialisedResource::CSerialisedResource(const CSerialisedResource& rhs) : CBaseCounted(),
Descriptor(rhs.Descriptor), LoosePath(rhs.LoosePath), Data(rhs.Data)
{

}

CSerialisedResource::CSerialisedResource(const CResourceDescriptorBase& desc) :
CBaseCounted(), Descriptor(desc), LoosePath(), Data()
{

}

CHash CResourceDescriptorBase::LatestHash() const
{
    if (!GUID) return Hash;
    const CFileDBRow* row = FileDB::FindByGUID(GUID);
    return row != NULL ? row->GetHash() : CHash::Zero;
}