#include "ResourceGame.h"
#include "ResourceLevel.h"
#include "PartPhysicsWorld.h"
#include "thing.h"



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

CPlayer* RGame::GetPlayerFromIndex(u32 index)
{
    // lazy

    u32 count = *(u32*)(((char*)this) + 0x144);
    if (index >= count) return NULL;


    const char* data = *(char**)(((char*)this) + 0x140);
    data += (index * 0x240);

    return (CPlayer*)data;
}