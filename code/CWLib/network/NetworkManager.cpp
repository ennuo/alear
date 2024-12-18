#include "network/NetworkManager.h"

#include <hook.h>


MH_DefineFunc(CNetworkInputManager_GetLocalLeadersPlayerNumber, 0x0012d5b8, TOC0, EPlayerNumber, CNetworkInputManager*);
EPlayerNumber CNetworkInputManager::GetLocalLeadersPlayerNumber()
{
    return CNetworkInputManager_GetLocalLeadersPlayerNumber(this);
}