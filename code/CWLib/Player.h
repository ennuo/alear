#ifndef PLAYER_H
#define PLAYER_H

#include "ResourceLocalProfile.h"
#include "ResourceSyncedProfile.h"
#include "PlayerNumber.inl"

class CPlayer {
private:
    char Pad[0x84];
    // char Pad[0x8c]; - accurate
public:
    EPlayerNumber PlayerNumber;
    CP<RSyncedProfile> SyncedProfile;
    CP<RLocalProfile> LocalProfile;
};

#endif // PLAYER_H