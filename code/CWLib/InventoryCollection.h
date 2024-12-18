#ifndef INVENTORY_COLLECTION_H
#define INVENTORY_COLLECTION_H

#include <vector.h>

#include "InventoryView.h"
#include "network/NetworkUtilsNP.h"

class CInventoryCollection : public CBaseCounted, public CReflectionVisitable {
public:
    CVector<CP<CInventoryView> > InventoryViews;
    NetworkPlayerID PlayerID;
    u32 CurrentPageNumber;
    u32 CollectionID;
    u32 ActionOnItemSelect;
    bool MenuFunctionalityLimited;
};


#endif // INVENTORY_COLLECTION_H