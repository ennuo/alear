#include <InventoryItem.h>
#include <ProfileCache.h>
#include <Slot.h>
#include <Translate.h>
#include <ResourceLocalProfile.h>
#include <ResourcePlan.h>
#include <PoppetEnums.inl>


CInventoryItemDetails::CInventoryItemDetails() : CDependencyWalkable(), HighlightSound(),
LevelUnlockSlotID(), PrimaryIndex(), LastUsed(), NumUses(), DateAdded(), FluffCost(),
CategoryIndex(-1), LocationIndex(-1), SubcategoryIndex(-1), Type(0), SubType(0), ToolType(0), Colour(c32::White),
Creator(INVALID_PLAYER_ID), Copyright(), AllowEmit(), Shareable(), NameTranslationTag(), DescTranslationTag(),
UserCreatedName(), UserCreatedDescription(), EditorList(), EyetoyData(), Icon(), PhotoData(), LoreTranslationTag(), Flags()
{
}

CInventoryItemDetails::CInventoryItemDetails(const CInventoryItemDetails& rhs) : CDependencyWalkable(), HighlightSound(),
LevelUnlockSlotID(), Colour(c32::White), UserCreatedName(), UserCreatedDescription(),
EditorList(), EyetoyData(), Icon(), PhotoData()
{
    *this = rhs;
}

CInventoryItemDetails::~CInventoryItemDetails()
{
    if (EyetoyData != NULL) delete EyetoyData;
    if (PhotoData != NULL) delete PhotoData;
}

CInventoryItemDetails& CInventoryItemDetails::operator=(const CInventoryItemDetails& rhs)
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
    EditorList = rhs.EditorList;
    UserCreatedDescription = rhs.UserCreatedDescription;
    UserCreatedName = rhs.UserCreatedName;
    DateAdded = rhs.DateAdded;

    if (rhs.EyetoyData != NULL)
        SetEyetoyData(*rhs.EyetoyData);
    if (rhs.PhotoData != NULL)
        SetPhotoData(*rhs.PhotoData);

    LoreTranslationTag = rhs.LoreTranslationTag;
    SubcategoryIndex = rhs.SubcategoryIndex;
    Flags = rhs.Flags;
    
    return *this;
}

void CInventoryItemDetails::SetEyetoyData(const CEyetoyData& eyetoy)
{
    if (EyetoyData == NULL) EyetoyData = new CEyetoyData();
    *EyetoyData = eyetoy;
}

void CInventoryItemDetails::SetPhotoData(const SInventoryItemPhotoData& data)
{
    if (PhotoData == NULL) PhotoData = new SInventoryItemPhotoData();
    *PhotoData = data;
}

MH_DefineFunc(CInventoryItemDetails_IsCreatedBy, 0x002ecef8, TOC0, bool, const CInventoryItemDetails*, const NetworkPlayerID&);
bool CInventoryItemDetails::IsCreatedBy(const NetworkPlayerID& pid) const
{
    return CInventoryItemDetails_IsCreatedBy(this, pid);
}

const tchar_t* CInventoryItemDetails::TranslateNameInternal()
{
    if (LevelUnlockSlotID.Empty())
    {
        if (NameTranslationTag == 0) return UserCreatedName.c_str();
        return Translate(NameTranslationTag);
    }

    CSlot* slot = ProfileCache::GetOrCreateMainUserProfile()->FindSlot(LevelUnlockSlotID);
    if (slot != NULL)
    {
        const tchar_t* name = slot->GetTranslatedName();
        if (name != NULL && *name != '\0')
            return name;
    }

    return Translate(0x596e4e1eul);
}

const tchar_t* CInventoryItemDetails::TranslateName()
{
    const tchar_t* name = TranslateNameInternal();
    if (name != NULL && *name != '\0')
        return name;

    if (Type & E_TYPE_COSTUME)
    {
        if (SubType == E_SUBTYPE_MORPH)
            return (const tchar_t*)L"A Morph";
        return Translate(0x685b6caul);
    }

    if (Type & E_TYPE_USER_STICKER)
        return Translate(0xa194da4ul);

    if (Type & E_TYPE_USER_POD)
        return Translate(0xdca8b69bul);
    
    return Translate(0x304d6601ul);
}

bool CInventoryItemDetails::IsUserCreated() const
{
    if (ToolType == TOOL_NOT_A_TOOL)
        return (Type & gUserObjectMask) != 0;
    return false;
}

u32 CInventoryItemDetails::GetAllocatedSize() const
{
    return 0;
}

void CInventoryItemDetails::SetIcon(RTexture* icon)
{
    Icon = icon;
}

void CInventoryItem::SetPlan(const CP<RPlan>& plan, const CP<CBaseProfile>& profile)
{
    plan->GetLoadDescriptor(Plan);
    Details = plan->InventoryData;

    u32 category = plan->InventoryData.GetCategory();
    if ((Details.Type & E_TYPE_COSTUME_MATERIAL) != 0)
        category = 2988363256ul;

    Details.SetLookupIndices(
        profile->AddString(plan->InventoryData.GetLocation()),
        profile->AddString(category)
    );
    Details.SetSubcategoryIndex(profile->AddString(plan->InventoryData.GetSubcategory()));
}

