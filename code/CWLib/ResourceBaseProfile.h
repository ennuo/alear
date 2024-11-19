#ifndef RESOURCE_BASE_PROFILE_H
#define RESOURCE_BASE_PROFILE_H

#include "Resource.h"
#include "ResourceDescriptor.h"
#include "ResourcePlan.h"
#include "InventoryItem.h"

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
};


#endif // RESOURCE_BASE_PROFILE_H