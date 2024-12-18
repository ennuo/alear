#include "ResourcePlan.h"
#include "thing.h"

MH_DefineFunc(RPlan_MakeClone, 0x000bfff8, TOC0, CThing*, RPlan*, PWorld*, NetworkPlayerID&, bool);
CThing* RPlan::MakeClone(RPlan* plan, PWorld* world, NetworkPlayerID& default_creator, bool remap_uids)
{
    return RPlan_MakeClone(plan, world, default_creator, remap_uids);
}

void CPlanDetails::SetIcon(RTexture* icon)
{
    CInventoryItemDetails::SetIcon(icon);
    PinnedIcon = icon;
}