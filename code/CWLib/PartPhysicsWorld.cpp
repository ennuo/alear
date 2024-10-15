#include "PartPhysicsWorld.h"

#include "hook.h"

MH_DefineFunc(PWorld_FindThingByScriptName, 0x0002e18c, TOC0, CThing*, PWorld*, char const*);
CThing* PWorld::FindThingByScriptName(char const* name)
{
    return PWorld_FindThingByScriptName(this, name);
}
