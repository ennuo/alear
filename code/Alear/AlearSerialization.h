#ifndef ALEAR_SERIALIZATION_H
#define ALEAR_SERIALIZATION_H

#include <StringUtil.h>
#include <mem_allocator.h>
#include <mem_stl_buckets.h>

#include <map>

struct SCompareCaseSensitive {
	bool operator()(const char* a, const char* b) const
	{
		return StringCompare(a, b) < 0;
	}
};

typedef std::map<const char*, int, SCompareCaseSensitive, STLBucketAlloc<std::pair<const char*, int> > > tGatherElementMap;

void SetGatherIndexLookup(tGatherElementMap* map);

class CScopedGatherLookup {
public:
    CScopedGatherLookup(tGatherElementMap* map)
    {
        SetGatherIndexLookup(map);
    }

    ~CScopedGatherLookup()
    {
        SetGatherIndexLookup(NULL);
    }
};

void AttachSerializationHooks();

#endif // ALEAR_SERIALIZATION_H