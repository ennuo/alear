#include "PartPhysicsWorld.h"

#include "hook.h"

MH_DefineFunc(PWorld_GetThingByUID, 0x0002524c, TOC0, CThing*, PWorld*, u32);
CThing* PWorld::GetThingByUID(u32 uid)
{
    return PWorld_GetThingByUID(this, uid);
}

MH_DefineFunc(PWorld_FindThingByScriptName, 0x0002e18c, TOC0, CThing*, PWorld*, char const*);
CThing* PWorld::FindThingByScriptName(char const* name)
{
    return PWorld_FindThingByScriptName(this, name);
}

MH_DefineFunc(PWorld_UpgradeAllScripts, 0x0002d7a4, TOC0, void, PWorld*);
void PWorld::UpgradeAllScripts()
{
    PWorld_UpgradeAllScripts(this);
}