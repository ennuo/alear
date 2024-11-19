#ifndef RESOURCE_PLAN_H
#define RESOURCE_PLAN_H

#include "Resource.h"
#include "ResourceGFXTexture.h"
#include "InventoryItem.h"
#include "vector.h"
#include "refcount.h"

class CThing;

class CPlanDetails : public CInventoryItemDetails {
public:
    u32 Location;
    u32 Category;
    CP<RTexture> PinnedIcon;
};

class RPlan : public CResource {
public:
    static CThing* MakeClone(RPlan* plan, PWorld* world, NetworkPlayerID& default_creator, bool remap_uids);
public:
    CThing* AssignedThing;
    u32 Revision;
    u32 BranchDescription;
    u8 CompressionFlags;
    ByteArray ThingData;
    CPlanDetails InventoryData;
};

#endif // RESOURCE_PLAN_H