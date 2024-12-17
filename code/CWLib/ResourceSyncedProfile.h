#ifndef RESOURCE_SYNCED_PROFILE_H
#define RESOURCE_SYNCED_PROFILE_H

#include <MMString.h>

#include "Resource.h"
#include "PlayerColours.h"
#include "Avatar.h"
#include "network/NetworkUtilsNP.h"

class RSyncedProfile : public CResource { // file.h: 26
private:
    CPlatonicAvatar PlatonicAvatar;
    CPlayerColours Colours;
    NetworkPlayerID PlayerID;
    u32 TimePlayed;
    u64 UniqueNumber;
private:
    void InitializeExtraData();
public:
    // Custom fields
    u32 StyleFlags;
    u32 StyleID;
    MMString<char> AnimationStyle;
};


#endif // RESOURCE_SYNCED_PROFILE_H