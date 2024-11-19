#ifndef RESOURCE_LOCAL_PROFILE_H
#define RESOURCE_LOCAL_PROFILE_H

#include "ResourceBaseProfile.h"
#include "SlotID.h"

class RLocalProfile : public CBaseProfile {
public:
    virtual bool AddInventoryItem(const CP<RPlan>& plan, u32 flags, int tutorial_video_enum_override, bool shareable, bool local_create, const CSlotID* level_from);
};


#endif // RESOURCE_LOCAL_PROFILE_H