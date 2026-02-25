#ifndef SERIALISEDRESOURCE_H
#define SERIALISEDRESOURCE_H

#include <filepath.h>
#include <vector.h>
#include <refcount.h>

#include "ResourceDescriptor.h"

class CSerialisedResource : public CBaseCounted {
public:
    CSerialisedResource();
    CSerialisedResource(const CResourceDescriptorBase& desc);
    CSerialisedResource(const CSerialisedResource& rhs);
public:
    const CResourceDescriptorBase& GetDescriptor() const { return Descriptor; }
    // ReflectReturn LoadCompressedBytes(CReflectionLoadVector& r);
private:
	CResourceDescriptorBase Descriptor;
	CFilePath LoosePath;
	ByteArray Data;
};

#endif