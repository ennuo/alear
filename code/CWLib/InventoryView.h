#ifndef INVENTORY_VIEW_H
#define INVENTORY_VIEW_H

#include "mmtypes.h"
#include "refcount.h"
#include "vector.h"

#include "MMString.h"
#include "ReflectionVisitable.h"

class CViewDescriptor {
public:
    u32 Type;
    u32 SubType;
    MMString<tchar_t> Title;
};

class CInventoryView : public CBaseCounted, CReflectionVisitable {
public:
    CViewDescriptor Descriptor;
    u32 CustomID;
    bool HeartedOnly;
    u32 CurrentSortMode;
    u32 DesiredSortMode;
    bool ContentsOutOfDate;
    bool UIOutOfDate;

    // CRawVector<SInventoryItemData> PageData
    // Cvector<SSortTermBoundary> SortTermBoundaries
    // CRawVector<u32> CategoryOrder
};

#endif // INVENTORY_VIEW_H