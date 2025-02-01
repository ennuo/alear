#include "Poppet.h"
#include "hook.h"

MH_DefineFunc(CPoppet_GetBubbleSize, 0x00343dc4, TOC1, v2, CPoppet*);
MH_DefineFunc(CPoppet_RenderHoverObject, 0x00344ab4, TOC1, void, CPoppet*, CThing*, float);
MH_DefineFunc(CPoppet_GetMode, 0x0033efa8, TOC1, EPoppetMode, const CPoppet*);
MH_DefineFunc(CPoppet_GetSubMode, 0x0033f22c, TOC1, EPoppetSubMode, const CPoppet*);
MH_DefineFunc(CPoppet_IsDocked, 0x00343bd8, TOC1, bool, const CPoppet*);
MH_DefineFunc(CPoppet_GetDockPos, 0x0033f03c, TOC1, v4hack, const CPoppet*);
MH_DefineFunc(CPoppet_GetLocalProfile, 0x0033e9f0, TOC1, const CP<RLocalProfile>&, const CPoppet*);
MH_DefineFunc(CPoppet_GetThingToIgnore, 0x00352f1c, TOC1, CThing*, CPoppet*);
MH_DefineFunc(CPoppet_SendPoppetDangerMessage, 0x0033fc1c, TOC1, void, CPoppet*, ELethalType);
MH_DefineFunc(CPoppet_PushMode, 0x0034dd08, TOC1, void, CPoppet*, EPoppetMode, EPoppetSubMode);
MH_DefineFunc(CPoppet_SendPoppetMessage, 0x0033fee0, TOC1, void, CPoppet*, EPoppetMessageType);
MH_DefineFunc(CPoppet_RenderUI, 0x0034584c, TOC1, void, CPoppet*);
MH_DefineFunc(CPoppet_SetDangerType, 0x003482e0, TOC1, void, CPoppet*, CThing*);

void CPoppet::SetDangerType(CThing* thing)
{
    CPoppet_SetDangerType(this, thing);
}

void CPoppet::RenderUI()
{
    CPoppet_RenderUI(this);
}

v2 CPoppet::GetBubbleSize()
{
    return CPoppet_GetBubbleSize(this);
}

void CPoppet::RenderHoverObject(CThing* thing, float outline)
{
    CPoppet_RenderHoverObject(this, thing, outline);
}

EPoppetMode CPoppet::GetMode() const
{
    return CPoppet_GetMode(this);
}

EPoppetSubMode CPoppet::GetSubMode() const
{
    return CPoppet_GetSubMode(this);
}

bool CPoppet::IsDocked() const
{
    return CPoppet_IsDocked(this);
}

v4 CPoppet::GetDockPos() const
{
    return CPoppet_GetDockPos(this).V;
}

CThing* CPoppet::GetThingToIgnore()
{
    return CPoppet_GetThingToIgnore(this);
}

const CP<RLocalProfile>& CPoppet::GetLocalProfile() const
{
    return CPoppet_GetLocalProfile(this);
}

void CPoppet::SendPoppetDangerMessage(ELethalType lethal_type)
{
    return CPoppet_SendPoppetDangerMessage(this, lethal_type);
}

void CPoppet::PushMode(EPoppetMode mode, EPoppetSubMode submode)
{
    return CPoppet_PushMode(this, mode, submode);
}

void CPoppet::SendPoppetMessage(EPoppetMessageType message)
{
    return CPoppet_SendPoppetMessage(this, message);
}

MH_DefineFunc(CPoppetInventory_TakePlan, 0x0038c590, TOC1, void, CPoppetInventory*, CVector<CThingPtr> const&);
void CPoppetInventory::TakePlan(CVector<CThingPtr> const& things)
{
    CPoppetInventory_TakePlan(this, things);
}