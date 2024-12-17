#ifndef INVENTORY_ITEM_H
#define INVENTORY_ITEM_H


#include "vector.h"

#include "ResourceDescriptor.h"
#include "ReflectionVisitable.h"
#include "GuidHash.h"
#include "SlotID.h"
#include "Colour.h"
#include "MMString.h"
#include "ResourcePointer.h"
#include "ResourceGFXTexture.h"
#include "InventoryItemFlags.h"

#include "network/NetworkUtilsNP.h"

class CInventoryItemDetails : public CDependencyWalkable {
public:
    inline void SetLookupIndices(u32 loc_idx, u32 cat_idx)
    {
        CategoryIndex = cat_idx;
        LocationIndex = loc_idx;
    }
public:
    CGUID HighlightSound;
    CSlotID LevelUnlockSlotID;
    u32 LocationIndex;
    u32 CategoryIndex;
    u32 PrimaryIndex;
    u32 LastUsed;
    u32 NumUses;
    union
    {
        u32 Pad;
        u32 CustomSortKey;
    };
    /* CalendarTime */ u64 DateAdded;
    int FluffCost;
    c32 Colour;
    u32 Type;
    u32 SubType;
    u32 ToolType;
    NetworkPlayerID Creator;
    bool AllowEmit;
    bool Shareable;
    bool Copyright;
    u32 NameTranslationTag;
    u32 DescTranslationTag;
    MMString<tchar_t> UserCreatedName;
    MMString<tchar_t> UserCreatedDescription;
    CVector<MMString<tchar_t> > EditorList;
    WR<RTexture> Icon;

    // dont need these structs right now
    void* PhotoData;
    void* EyetoyData;
public:
    bool IsCreatedBy(NetworkPlayerID* pid);
private:
    // meant to be SetIcon, just here to force the vtable to be generated
    virtual void Dummy() {}
};

class CInventoryItem {
public:
    CResourceDescriptor<RPlan> Plan;
    CInventoryItemDetails Details;
    u32 UID;
    /* ETutorialLevels */ u32 TutorialLevel;
    /* ETutorialLevels */ u32 TutorialVideo;
    u32 Flags;
};


#endif // INVENTORY_ITEM_H