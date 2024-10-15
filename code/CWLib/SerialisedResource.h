#ifndef SERIALISEDRESOURCE_H
#define SERIALISEDRESOURCE_H

#include <filepath.h>
#include <vector.h>
#include <refcount.h>

#include "ResourceDescriptor.h"

class CSerialisedResource : public CBaseCounted {
private:
	CResourceDescriptorBase Descriptor;
	CFilePath LoosePath;
	ByteArray Data;
};

#endif