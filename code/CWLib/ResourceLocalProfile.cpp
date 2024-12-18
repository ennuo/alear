#include "ResourceLocalProfile.h"
#include <hook.h>

const u32 gUsedItemsCustomId = 0x55534544;

bool RLocalProfile::IsWearingCostumeUID(u32 uid) const
{
    for (u32* it = CostumeUIDList.begin(); it != CostumeUIDList.end(); ++it)
    {
        if (*it == uid) 
            return true;
    }
    
    return false;
}

MH_DefineFunc(RLocalProfile_SetViewsDirtyIfTheyContainItem, 0x0009ce78, TOC0, void, RLocalProfile*, u32 uid);
void RLocalProfile::SetViewsDirtyIfTheyContainItem(u32 uid)
{
    RLocalProfile_SetViewsDirtyIfTheyContainItem(this, uid);
}

void RLocalProfile::SetUsedItemViewDirty()
{
    for (int i = 0; i < InventoryViews.size(); ++i)
    {
        CP<CInventoryView>& view = InventoryViews[i];
        if (view && view->Descriptor.Type == gUsedItemsCustomId)
            view->ContentsOutOfDate = true;
    }
}