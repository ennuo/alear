#ifndef RESOURCE_LOCAL_PROFILE_H
#define RESOURCE_LOCAL_PROFILE_H

#include <refcount.h>

#include "ResourceDescriptor.h"
#include "ResourceBaseProfile.h"
#include "SlotID.h"
#include "InventoryView.h"
#include "InventoryCollection.h"
#include "network/NetworkUtilsNP.h"

#define MAX_USER_EMOTES (4)

extern const u32 gUsedItemsCustomId;
extern const u32 gEmotesCustomId;

class CInventoryTemplateLevel {
public:
    inline CInventoryTemplateLevel() : Level(), DateAdded(), NameTranslationTag(), UserCreatedName(), Creator(INVALID_PLAYER_ID)
    {}
public:
    CResourceDescriptor<RLevel> Level;
    u64 DateAdded;
    u32 NameTranslationTag;
    MMString<tchar_t> UserCreatedName;
    NetworkPlayerID Creator;
};

// 0x3b8
class RLocalProfile : public CBaseProfile {
public:
    static CP<RLocalProfile> EMPTY;
public:
    bool IsWearingCostumeUID(u32 uid) const;
    void SetViewsDirtyIfTheyContainItem(u32 uid);
public:
    virtual bool AddInventoryItem(const CP<RPlan>& plan, u32 flags, int tutorial_video_enum_override, bool shareable, bool local_create, const CSlotID* level_from);
private:
    void InitializeExtraData();
public:
    void SetUsedItemViewDirty();
public:
    CRawVector<void*> PoppetModeStack; // meant to be a vec of CPoppetMode, just dont care rn
    u32 NextInventoryCollectionID;
    CRawVector<u32> CostumeUIDList;
    CVector<void*> InventoryViewBackup;
    CVector<CP<CInventoryView> > InventoryViews;
    CVector<CP<CInventoryCollection> > InventoryViewCollections;
private:
    char Pad[0x2e0];
public:
    CVector<u32> HiddenCategories;
    CResourceDescriptor<RPlan> Emotes[MAX_USER_EMOTES];
    CResourceDescriptor<RPlan> SelectedAnimationStyle;
    CVector<CInventoryTemplateLevel> LevelTemplates;
};


#endif // RESOURCE_LOCAL_PROFILE_H