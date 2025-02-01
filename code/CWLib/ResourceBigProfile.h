#ifndef RESOURCE_BIG_PROFILE_H
#define RESOURCE_BIG_PROFILE_H

#include "ResourceBaseProfile.h"
#include "poppet/PoppetLimits.h"


class RLocalProfile;

class RBigProfile : public CBaseProfile {
public:
    SlotMap MyMoonSlots;
    RLocalProfile* LocalProfile;
    CInventoryLimitsHaver Limits;
};

#endif // RESOURCE_BIG_PROFILE_H