#include "ResourceGame.h"
#include <hook.h>

MH_DefineFunc(RGame_GetYellowheadFromPlayerNumber, 0x000934e4, TOC0, CThing*, RGame*, EPlayerNumber);
CThing* RGame::GetYellowheadFromPlayerNumber(EPlayerNumber player_number)
{
    return RGame_GetYellowheadFromPlayerNumber(this, player_number);
}