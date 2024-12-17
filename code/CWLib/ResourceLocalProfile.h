#ifndef RESOURCE_LOCAL_PROFILE_H
#define RESOURCE_LOCAL_PROFILE_H

#include "ResourceDescriptor.h"
#include "ResourceBaseProfile.h"
#include "SlotID.h"

#define MAX_USER_EMOTES (4)

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
    CRawVector<void*> PoppetModeStack; // meant to be a vec of CPoppetMode, just dont care rn
    u32 NextInventoryCollectionID;
    CRawVector<u32> CostumeUIDList;
private:
    char Pad[0x304];
public:
    CVector<u32> HiddenCategories;
    CResourceDescriptor<RPlan> Emotes[MAX_USER_EMOTES];
    CResourceDescriptor<RPlan> SelectedAnimationStyle;
};


#endif // RESOURCE_LOCAL_PROFILE_H