#ifndef RESOURCE_PLAN_H
#define RESOURCE_PLAN_H

#include "Resource.h"
#include "ResourceGFXTexture.h"
#include "InventoryItem.h"
#include "thing.h"
#include "vector.h"
#include "refcount.h"

class CPlanDetails : public CInventoryItemDetails {
public:
    u32 Location;
    u32 Category;
    CP<RTexture> PinnedIcon;
};

class RPlan : public CResource {
public:
    CThing* AssignedThing;
    u32 Revision;
    u32 BranchDescription;
    u8 CompressionFlags;
    ByteArray ThingData;
    CPlanDetails InventoryData;
};

#endif // RESOURCE_PLAN_H