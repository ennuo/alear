#include "ResourceGame.h"
#include "ResourceLevel.h"
#include "PartPhysicsWorld.h"
#include "thing.h"

#include <hook.h>

MH_DefineFunc(RGame_TeleportPlayer, 0x000aa008, TOC0, void, RGame*, CThing*, v2 const&);
void RGame::TeleportPlayer(CThing* player, v2 const& pos)
{
    RGame_TeleportPlayer(this, player, pos);
}

MH_DefineFunc(RGame_GetYellowheadFromPlayerNumber, 0x000934e4, TOC0, CThing*, RGame*, EPlayerNumber);
CThing* RGame::GetYellowheadFromPlayerNumber(EPlayerNumber player_number)
{
    return RGame_GetYellowheadFromPlayerNumber(this, player_number);
}

PWorld* RGame::GetWorld()
{
    if (!Level) return NULL;
    return Level->GetWorld();
}