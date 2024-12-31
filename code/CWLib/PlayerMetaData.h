#ifndef PLAYER_METADATA_H
#define PLAYER_METADATA_H

#include <vector.h>

#include "network/NetworkUtilsNP.h"
#include "PhotoMetaData.h"
#include "SlotID.h"

#define MAX_PINS (3)

struct PlagachefSkill { // file.h: 10
    u32 Rank;
    u32 Value;
};

struct SPlayerMetaData { // file.h: 20
    v3 Location;
    CRawVector<CSlotID> FavouriteLevels;
    CRawVector<SceNpOnlineId> FavouritePlayers;
    CVector<SServerPhotoData> Photos;

    // adding pin support, but it's a bit much effort to
    // adjust the size of this struct, so sneaking pins
    // as a replacement field for plagachef since it was removed
    // from later releases.
    union
    {
        struct {
            PlagachefSkill PlaySkill;
            PlagachefSkill CreateSkill;
            PlagachefSkill ShareSkill;
        };

        struct {
            u32 Pins[MAX_PINS];
        };
    };

    CSlotID RootPlaylist;
    u32 EntitledSlots;
    u32 UsedSlots;
    u32 HeartCount;
};

#endif // PLAYER_METADATA_H