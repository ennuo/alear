#pragma once

#include <Resource.h>
#include <InventoryItem.h>
#include <vector.h>

class CThing;

class CPlanDetails : public CInventoryItemDetails {
public:
    inline CPlanDetails() : CInventoryItemDetails(), 
    Location(), Category(), PinnedIcon(), Subcategory()
    {

    }
public:
    inline u32 GetLocation() const { return Location; }
    inline u32 GetCategory() const { return Category; }
    inline u32 GetSubcategory() const { return Subcategory; }
public:
    inline void SetLocation(u32 location) { Location = location; }
    inline void SetCategory(u32 category) { Category = category; }
    inline void SetSubcategory(u32 subcategory) { Subcategory = subcategory; }
public:
    void SetIcon(RTexture* icon);
    void Refresh(CThing*);
public:
    u32 Location;
    u32 Category;
    CP<RTexture> PinnedIcon;
    u32 Subcategory;
};

class RPlan : public CResource {
public:
    void InitializeExtraData();
public:
    static CThing* MakeClone(RPlan* plan, PWorld* world, NetworkPlayerID& default_creator, bool remap_uids);
protected:
    CThing* AssignedThing;
public:
    u32 Revision;
    u32 BranchDescription;
    u8 CompressionFlags;
    ByteArray ThingData;
    CPlanDetails InventoryData;
};
