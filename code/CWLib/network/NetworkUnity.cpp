#include <hook.h>
#include "network/NetworkPartiesData.h"

MH_DefineFunc(CGamePartyData_NumPS3s, 0x00129330, TOC0, u32, CGamePartyData const*);
u32 CGamePartyData::NumPS3s() const
{
    return CGamePartyData_NumPS3s(this);
}

CGamePartyData& GetGamePartyData(ECheckThread check_thread)
{
    return gGamePartyData;
}