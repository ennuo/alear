#pragma once

#include <ResourceDescriptor.h>
#include <ReflectionVisitable.h>
#include <GuidHash.h>
#include <SlotID.h>
#include <Colour.h>
#include <MMString.h>
#include <ResourcePointer.h>
#include <ResourceGFXTexture.h>
#include <InventoryItemFlags.h>
#include <CalendarTime.h>
#include <ColorCorrection.h>
#include <PhotoMetaData.h>
#include <network/NetworkUtilsNP.h>
#include <PoppetEnums.inl>
#include <vector.h>

const u32 gUserObjectMask = 0x40100480;
const u32 gPodMask = E_TYPE_USER_POD | E_TYPE_POD_TOOL;

class CBaseProfile;
class CThing;

class CInventoryItemDetails : public CDependencyWalkable {
public:
    inline const CGUID& GetHighlightSound() const { return HighlightSound; }
    inline int GetFluffCost() const { return FluffCost; }
    inline u32 GetType() const { return Type; }
    inline u32 GetSubType() const { return SubType; }
    inline EToolType GetToolType() const { return (EToolType)ToolType; }
    inline bool GetAllowEmit() const { return AllowEmit; }
    inline const CSlotID& GetLevelUnlockSlotID() const { return LevelUnlockSlotID; }
    inline u32 GetName() const { return NameTranslationTag; }
    inline u32 GetDescription() const { return DescTranslationTag; }
    inline u32 GetLore() const { return LoreTranslationTag; }
public:
    const tchar_t* TranslateName();
    const tchar_t* TranslateDescription();
public:
    inline const CVector<MMString<tchar_t> >& GetEditorList() const { return EditorList; }
public:
    bool IsUserCreated() const;
public:
    inline const tchar_t* GetUserCreatedName() const { return UserCreatedName.c_str(); }
    inline const tchar_t* GetUserCreatedDescription() const { return UserCreatedDescription.c_str(); }
public:
    bool IsManuallySetItem() const;
public:
    inline v4 GetColour() const { return Colour.AsV4(); }
    inline CEyetoyData* GetEyetoyData() const { return EyetoyData; }
    inline SInventoryItemPhotoData* GetPhotoData() const { return PhotoData; }
    inline CalendarTime GetDateAdded() const { return DateAdded; }
public:
    bool IsCopyrightedToMe(const NetworkPlayerID& player) const;
    bool IsCopyrightedToNotMe(const NetworkPlayerID& player) const;
    bool IsCopyrightedToMe(CThing*) const;
    bool IsCopyrightedNotToMe(CThing*) const;
public:
    inline void SetCopyrightFlag(bool copyright) { Copyright = copyright; }
    inline bool GetCopyrightFlag() const { return Copyright; }
public:
    inline void SetLevelUnlockSlotID(const CSlotID& id) { LevelUnlockSlotID = id; }
    inline void SetType(u32 type) { Type = type; }
    inline void SetSubType(u32 subtype) { SubType = subtype; }
    inline void SetAllowEmit(bool allow_emit) { AllowEmit = allow_emit; }
    inline void SetFluffCost(int fluff_cost) { FluffCost = fluff_cost; }
public:
    void CompleteType(CThing*);
    void CompleteSubType(CThing*);
    void SetTypeFromThing(CThing*);
public:
    inline const NetworkPlayerID& GetCreator() const { return Creator; }
    inline void SetCreator(const NetworkPlayerID& creator) { Creator = creator; }
public:
    bool IsCreatedBy(const NetworkPlayerID& pid) const;
    bool IsCreatorUnknownOrEmpty() const;
    bool IsCreatorUnknown() const;
    bool IsCreatorEmpty() const;
public:
    inline EInventoryItemFlags GetFlags() const { return (EInventoryItemFlags)Flags; }
    inline u32 GetNumUses() const { return NumUses; }
    inline u32 GetLocationIndex() const { return LocationIndex; }
    inline u32 GetCategoryIndex() const { return CategoryIndex; }
    inline u32 GetPrimaryIndex() const { return PrimaryIndex; }
    inline u32 GetSubcategoryIndex() const { return SubcategoryIndex; }
public:
    inline void IncrementUsage() { NumUses++; }
public:
    void SetDateAddedToNow();
    virtual u32 GetAllocatedSize() const;
    virtual void SetIcon(RTexture* icon);
public:
    inline void SetHiglightSound(const CGUID& sound) { HighlightSound = sound; }
public:
    inline void SetLookupIndices(u32 loc_idx, u32 cat_idx)
    {
        CategoryIndex = cat_idx;
        LocationIndex = loc_idx;
    }

    inline void SetPrimaryIndex(u32 index) { PrimaryIndex = index; }
    inline void SetSubcategoryIndex(u32 index) { SubcategoryIndex = index; }
public:
    void SetEyetoyData(const CEyetoyData&);
    void SetPhotoData(const SInventoryItemPhotoData&);
public:
    inline void SetUserCreatedName(const tchar_t* name) { UserCreatedName = name; }
    inline void SetUserCreatedDescription(const tchar_t* description) { UserCreatedDescription = description; }
    inline void SetName(u32 key) { NameTranslationTag = key; }
    inline void SetLore(u32 key) { LoreTranslationTag = key; }
    inline void SetDescription(u32 key) { DescTranslationTag = key; }
    inline void SetToolInfo(u32, u32, u32, u32) {}
public:
    void Refresh();
public:
    inline bool IsAPrimitiveShape() const { return Type == E_TYPE_PRIMITIVE_SHAPE; }
    inline bool IsAPrimitiveMaterial() const { return Type == E_TYPE_PRIMITIVE_MATERIAL; }
    inline bool IsAPrimitive() const { return IsAPrimitiveMaterial() || IsAPrimitiveShape(); }
    inline bool IsADecoration() const { return Type == E_TYPE_DECORATION; }
    inline bool IsASticker() const { return (Type & E_TYPE_STICKER) != 0; }
    inline bool IsACostume() const { return Type == E_TYPE_USER_COSTUME || Type == E_TYPE_COSTUME; }
    inline bool IsACostumeMaterial() const { return Type == E_TYPE_COSTUME_MATERIAL; }
    inline bool IsAJoint() const { return Type == E_TYPE_JOINT; }
    inline bool IsAGameplayKit() const { return Type == E_TYPE_GAMEPLAY_KIT; }
    inline bool IsABackground() const { return Type == E_TYPE_BACKGROUND; }
    inline bool IsEyetoy() const { return Type == E_TYPE_EYETOY; }
    inline bool IsATool() const { return ToolType != TOOL_NOT_A_TOOL; }
    inline bool IsAPlayerColour() const { return Type == E_TYPE_PLAYER_COLOUR; }
    inline bool IsFloodFill() const { return Type == E_TYPE_FLOOD_FILL; }
    inline bool IsAGadget() const { return Type == E_TYPE_GADGET; }
    inline bool IsAMusic() const { return Type == E_TYPE_MUSIC; }
    inline bool IsASound() const { return Type == E_TYPE_SOUND; }
    inline bool IsAPod() const { return Type == E_TYPE_USER_POD; }
public:
    inline bool IsAnimated() const { return (Flags & E_ITEM_ANIMATED) != 0; }
    inline bool IsColorable() const { return (Flags & E_ITEM_USER_COLOR) != 0; }
    inline bool IsLocked() const { return (Flags & E_ITEM_LOCKED) != 0; }
public:
    bool CreateInLimbo();
public:
    CInventoryItemDetails();
    virtual ~CInventoryItemDetails();
    CInventoryItemDetails(const CInventoryItemDetails&);
    CInventoryItemDetails& operator=(CInventoryItemDetails const&);
private:
    const tchar_t* TranslateNameInternal();
public:
    CGUID HighlightSound;
    CSlotID LevelUnlockSlotID;
    u32 LocationIndex;
    u32 CategoryIndex;
    u32 PrimaryIndex;
    u32 LastUsed;
    u32 NumUses;
    u32 SubcategoryIndex;
    CalendarTime DateAdded;
    int FluffCost;
    c32 Colour;
    u32 Type;
    u32 SubType;
    u32 ToolType;
    NetworkPlayerID Creator;
    bool AllowEmit;
    bool Shareable;
    bool Copyright;
    u8 Flags;
    u32 NameTranslationTag;
    u32 DescTranslationTag;
    u32 LoreTranslationTag;
    MMString<tchar_t> UserCreatedName;
    MMString<tchar_t> UserCreatedDescription;
    CVector<MMString<tchar_t> > EditorList;
    WR<RTexture> Icon;
    SInventoryItemPhotoData* PhotoData;
    CEyetoyData* EyetoyData;
};

class CInventoryItem {
public:
    inline CInventoryItem() : Plan(), Details(), UID(), TutorialLevel(), TutorialVideo(), Flags() {}
public:
    inline u32 GetFlags() const { return Flags; }
    inline void SetFlags(u32 flags) { Flags = flags; }
    inline bool IsHearted() const { return (Flags & E_IIF_HEARTED) != 0; }
    inline bool IsUploaded() const { return (Flags & E_IIF_UPLOADED) != 0; }
    inline bool IsCheat() const { return (Flags & E_IIF_CHEAT) != 0; }
    inline bool IsUnsaved() const { return (Flags & E_IIF_UNSAVED) != 0; }
    inline bool IsErrored() const { return (Flags & E_IIF_ERRORED) != 0; }

    void SetPlan(const CP<RPlan>& plan, const CP<CBaseProfile>&);
public:
    CResourceDescriptor<RPlan> Plan;
    CInventoryItemDetails Details;
    u32 UID;
    /* ETutorialLevels */ u32 TutorialLevel;
    /* ETutorialLevels */ u32 TutorialVideo;
protected:
    u32 Flags;
};
