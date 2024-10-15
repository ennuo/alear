#include "InventoryItem.h"

#include "hook.h"

MH_DefineFunc(CInventoryItemDetails_IsCreatedBy, 0x002ecef8, TOC0, bool, CInventoryItemDetails*, NetworkPlayerID*);
bool CInventoryItemDetails::IsCreatedBy(NetworkPlayerID* pid)
{
    return CInventoryItemDetails_IsCreatedBy(this, pid);
}


