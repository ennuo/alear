#include "Poppet.h"
#include "hook.h"
#include "AlearConfig.h"

#include <cell/DebugLog.h>

#include "cell/DebugLog.h"
#include "AlearConfig.h"

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
MH_DefineFunc(CPoppet_FloodFill, 0x003401d8, TOC1, bool, CPoppet*, CThing*);
MH_DefineFunc(CPoppet_Backup, 0x0034b844, TOC1, void, CPoppet*);
MH_DefineFunc(CPoppet_ClearMarquee, 0x004077a0, TOC1, void, CPoppet*);

void CPoppet::SetDangerType(CThing* thing)
{
    CPoppet_SetDangerType(this, thing);
}

bool CPoppet::FloodFill(CThing* thing)
{
    return CPoppet_FloodFill(this, thing);
}

void CPoppet::Backup()
{
    CPoppet_Backup(this);
}

void CPoppet::ClearMarquee()
{
    CPoppet_ClearMarquee(this);
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

bool CanScaleMesh(CGUID mesh_guid)
{
    if (gForceMeshScaling) return true;

    DebugLog("Checking if we can scale g%08x\n", mesh_guid.guid);

    switch (mesh_guid.guid)
    {
        // Sackboy
        case 0x43f:
        // Spawn Points
        // Wood
        case 0x2d94:
        case 0x4009:
        case 0x7a06:
        case 0xd89f:
        case 0x10b22:
        // Chrome
        case 0x1d1b1:
        case 0x1d1b4:
        case 0x1d1b5:
        case 0x1d86d:
        // Plastic
        case 0x1d1bb:
        case 0x1d1bc:
        case 0x1d1be:
        case 0x1d86b:
        // Cardboard
        case 0x1d859:
        case 0x1d853:
        case 0x1d867:
        case 0x1dd3f:
        // Link
        case 0xe91:
        // Race Kit
        case 0x2d40:
        case 0x2d86:
        // Player Limit Post
        case 0x99fb:
        // Scoreboards
        // Cardboard
        case 0x441c:
        // Wood
        // Chrome
        // Plastic
        // Video Screens
        case 0x67d0:
        // Pod
        case 0x2f6c:
        // Early Powerups
        case 0x8e1:
        case 0x94d:
        case 0x94e:
        case 0xb9a:
        // Powerup Pedestals
        case 0x1054a:
        case 0x106a0:
        case 0x1074d:
        case 0x10ad3:
        case 0x10ad5:
        case 0x1290c:
        case 0x13c88:
        case 0x13c86:
        case 0x13ed8:
        case 0x18fc1:
        case 0xc731f6f7:
        case 0x9d98bde9:
        case 0xc190b37e:
        case 0x88cc859b:
        case 0xbe8735f3:
            return false;
    }

    return true;
}
