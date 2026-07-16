#ifndef RESOURCE_GAME_H
#define RESOURCE_GAME_H

class RLevel;
class CThing;
class PWorld;
class CPlayer;

#include "Resource.h"
#include "PlayerNumber.inl"
#include <RandomStream.h>

class RGame : public CResource {
public:
    CThing* GetYellowheadFromPlayerNumber(EPlayerNumber player_number);
    void TeleportPlayer(CThing* player, v2 const& position);
    CPlayer* GetPlayerFromIndex(u32);
    CPlayer* GetPlayerFromPlayerNumber(EPlayerNumber player_number);

    inline CRandomStream& GetNonDeterministicRandStream()
    {
        return *(CRandomStream*)(((char*)this) + 0x1b8);
    }
    
public:
    PWorld* GetWorld();
public:
    u32 PauseState;
    float PauseT;
    float PauseAmbientT;
    u32 PauseTrigger;
    u32 SeekState;
    float SeekT;
    float SeekAmbientT;
    u32 SeekTrigger;
    float VCRDamp;
    float FadeTimeLeft;
    CP<RLevel> Level;
private:
    char Pad[0x90];
public:
    bool EditMode;
};

extern RGame* gGame;

#endif // RESOURCE_GAME_H