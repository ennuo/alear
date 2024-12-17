#ifndef INVENTORY_VIEW_H
#define INVENTORY_VIEW_H


#include "refcount.h"
#include "vector.h"

#include "MMString.h"
#include "ReflectionVisitable.h"

class CInventoryItem;
class CViewDescriptor {
public:
    u32 Type;
    u32 SubType;
    MMString<tchar_t> Title;
};


class SSortTermBoundary {
public:
    inline SSortTermBoundary() : Index(), Name()
    {

    }
public:
    u32 Index;
    MMString<wchar_t> Name;
};

class CInventoryView : public CBaseCounted, CReflectionVisitable {
public:
    struct SInventoryItemData {
        CInventoryItem* Item;
        u32 CurSortIndex;
    };
public:
    CViewDescriptor Descriptor;
    u32 CustomID;
    bool HeartedOnly;
    u32 CurrentSortMode;
    u32 DesiredSortMode;
    bool ContentsOutOfDate;
    bool UIOutOfDate;
    CRawVector<SInventoryItemData> PageData;
    CVector<SSortTermBoundary> SortTermBoundaries;
    CRawVector<u32> CategoryOrder;
};

#endif // INVENTORY_VIEW_H