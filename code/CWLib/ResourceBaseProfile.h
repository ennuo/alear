#ifndef RESOURCE_BASE_PROFILE_H
#define RESOURCE_BASE_PROFILE_H

#include <vector.h>

#include "Resource.h"
#include "ResourceDescriptor.h"
#include "ResourcePlan.h"
#include "InventoryItem.h"
#include "StringLookupTable.h"

class CBaseProfile : public CResource {
public:
    virtual void Clear();
    virtual void OnSaveProfileBegin();
    virtual void OnSaveProfileEnd(bool, bool);
    virtual void OnSerialiseProfileBegin();
    virtual void OnSerialiseProfileEnd();
    virtual bool HasInventoryItem(CResourceDescriptor<RPlan> const& plan);
    virtual CInventoryItem* GetInventoryItem(CResourceDescriptor<RPlan> const& plan);
    virtual CInventoryItem* GetInventoryItemByUID(u32 uid);
    virtual bool RemoveInventoryItem(u32 uid);
    virtual void ClearInventory();
    virtual void CopyProfile(CBaseProfile* profile);
    virtual void OnInventoryChanged();
public:
    u32 AddString(u32 key);
public:
    u8 Status;
    u32 NextInventoryItemUID;
    CStringLookupTable StringTable;
    CVector<CInventoryItem> Inventory;
    CRawVector<CInventoryItem*> SortedInventoryUID;
    CRawVector<CInventoryItem*> SortedInventoryPlanDescriptor;
};


#endif // RESOURCE_BASE_PROFILE_H