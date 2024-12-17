#ifndef RESOURCE_H
#define RESOURCE_H


#include "refcount.h"

#include "ReflectionVisitable.h"
#include "SerialiseEnums.h"
#include "ResourceDescriptor.h"
#include "SerialisedResource.h"
#include "ResourceEnums.h"
#include "GuidHash.h"
#include "Serialise.h"

extern u32 gLazyGCTime;

enum EResourceFlag {
	FLAG_REF_COUNT_DIRTY = 2,
	FLAG_CONTAINS_EYETOY = 128,
	FLAG_DONTSWIZZLE = 256,
	FLAG_VOLTEX = 512,
	FLAG_NOSRGB_TEX = 1024,
	FLAGS_BUMP_TEX = 2048,
	FLAGS_TEMPORARY = 8192,
	FLAGS_UNSHARED = 16384,
	FLAGS_MAX_MIP_128 = 65536
};

class CResource : public CReflectionVisitable, public CDependencyWalkable {
public:
    CResource(EResourceFlag flags, EResourceType type);
public:
    inline CGUID& GetGUID() { return GUID; }
    inline CHash& GetLoadedHash() { return LoadedHash; }
    inline EResourceType GetResourceType() const { return ResourceType; }
public:
    volatile u32 RefCount;
    volatile u32 WeakCount;
    EResourceLoadState LoadState;
    CStreamPriority Priority;
    u32 Flags;
    u32 LazyGCTime;
    CP<CSerialisedResource> CSR;
protected:
    EResourceType ResourceType;
    CGUID GUID;
    CHash LoadedHash;
    u32 CachedSizeInMemory;
public:
    inline u32 GetRefCount() { return RefCount; }
    
    inline u32 AddRef() 
    {
        Flags |= FLAG_REF_COUNT_DIRTY;
        LazyGCTime = gLazyGCTime;
		return cellAtomicIncr32((uint32_t*) &this->RefCount);
	}

	inline u32 Release() 
    {
        Flags |= FLAG_REF_COUNT_DIRTY;
        LazyGCTime = gLazyGCTime;
		return cellAtomicDecr32((uint32_t*) &this->RefCount);
	}

    inline bool IsLoaded()
    {
        return LoadState == LOAD_STATE_LOADED;
    }

    inline bool IsError()
    {
        return LoadState >= LOAD_STATE_ERROR;
    }

    ReflectReturn Duplicate(CResource* resource);
    void BlockUntilLoaded();
public:
    virtual void Unload();
    virtual ReflectReturn LoadFinished(SRevision const& load_revision);
    virtual void AddDependencies(CVector<CP<CResource> >& dependencies);
    virtual u32 GetSizeInMemory();
    virtual u32 GetSizeInGfxMemoryPool(u32 location);
    virtual ~CResource();
};


#endif // RESOURCE_H