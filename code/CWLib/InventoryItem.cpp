#include "InventoryItem.h"

#include "hook.h"

CInventoryItemDetails::CInventoryItemDetails() : CDependencyWalkable(), HighlightSound(),
LevelUnlockSlotID(), PrimaryIndex(), LastUsed(), NumUses(), Pad(), DateAdded(), FluffCost(),
CategoryIndex(-1), LocationIndex(-1), Type(0), SubType(0), ToolType(0), Colour(c32::White),
Creator(INVALID_PLAYER_ID), Copyright(), AllowEmit(), Shareable(), NameTranslationTag(), DescTranslationTag(),
UserCreatedName(), UserCreatedDescription(), EditorList(), EyetoyData(), Icon(), PhotoData()
{
}

CInventoryItemDetails::CInventoryItemDetails(CInventoryItemDetails& rhs) : CDependencyWalkable(), HighlightSound(),
LevelUnlockSlotID(), Colour(c32::White), UserCreatedName(), UserCreatedDescription(),
EditorList(), EyetoyData(), Icon(), PhotoData()
{
    *this = rhs;
}

CInventoryItemDetails& CInventoryItemDetails::operator=(CInventoryItemDetails const& rhs)
{
    if (this == &rhs) return *this;

    ToolType = rhs.ToolType;
    SubType = rhs.SubType;
    NameTranslationTag = rhs.NameTranslationTag;
    DescTranslationTag = rhs.DescTranslationTag;
    HighlightSound = rhs.HighlightSound;
    LocationIndex = rhs.LocationIndex;
    CategoryIndex = rhs.CategoryIndex;
    PrimaryIndex = rhs.PrimaryIndex;
    LastUsed = rhs.LastUsed;
    NumUses = rhs.NumUses;
    FluffCost = rhs.FluffCost;
    Type = rhs.Type;
    AllowEmit = rhs.AllowEmit;
    Colour = rhs.Colour;
    Icon = rhs.Icon;
    Creator = rhs.Creator;
    LevelUnlockSlotID = rhs.LevelUnlockSlotID;
    Copyright = rhs.Copyright;
    Shareable = rhs.Shareable;

    // EditorList = rhs.EditorList;
    UserCreatedDescription = rhs.UserCreatedDescription;
    UserCreatedName = rhs.UserCreatedName;
    DateAdded = rhs.DateAdded;

    // if (rhs.EyetoyData != NULL)
    //     SetEyetoyData(rhs.EyetoyData);
    // if (rhs.PhotoData != NULL)
    //     SetPhotoData(rhs.PhotoData);

    return *this;
}

MH_DefineFunc(CInventoryItemDetails_IsCreatedBy, 0x002ecef8, TOC0, bool, CInventoryItemDetails*, NetworkPlayerID*);
bool CInventoryItemDetails::IsCreatedBy(NetworkPlayerID* pid)
{
    return CInventoryItemDetails_IsCreatedBy(this, pid);
}

void CInventoryItemDetails::SetIcon(RTexture* icon)
{
    Icon = icon;
}


