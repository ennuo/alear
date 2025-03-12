#ifndef NETWORK_PARTIES_DATA_H
#define NETWORK_PARTIES_DATA_H

#include <vector.h>

#include "network/NetworkUtilsNP.h"
#include "network/NetworkEnum.inl"

#include "PadIndex.inl"
#include "PlayerNumber.inl"

enum ECheckThread
{
    E_CHECK_THREAD,
    E_DEBUG_DONT_CHECK_THREAD
};

class CPadToPlayerNumber {
public:
    EPadIndex Pad;
    EPlayerNumber PlayerNumber;
    bool IsReservation;
};

class CPS3PartyData {
private:
    NetworkPlayerID PlayerID;
    NetworkPlayerID LeaderPlayerID;
    CRawVector<CPadToPlayerNumber> PlayerNumbers;
    u32 PadsConnectedBits;
    EMatchmakingMethod HowMatchmade;
};

class CGamePartyData {
public:
    u32 NumPS3s() const;
private:
    CPS3PartyData PS3sInGame[4];
};

extern CGamePartyData gGamePartyData;
CGamePartyData& GetGamePartyData(ECheckThread check_thread);

#endif // NETWORK_PARTIES_DATA_H