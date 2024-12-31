#include "customization/PodStyles.h"


#include "thing.h"
#include "cell/DebugLog.h"

#include "ResourceDescriptor.h"
#include "InventoryView.h"
#include "InventoryItem.h"
#include "network/NetworkUtilsNP.h"

#include "PoppetEnums.inl"
#include "ResourceGame.h"
#include "ResourceLevel.h"

#include "hook.h"
#include "ppcasm.h"

CThing* GetPodThing(PWorld* world)
{
    for (int i = 0; i < world->ListPRenderMesh.size(); ++i)
    {
        PRenderMesh* part = world->ListPRenderMesh[i];
        CThing* thing = part->GetThing();

        PPos* pos = thing->GetPPos();
        if (pos == NULL) continue;

        // unreliable in 2 onward!
        if (pos->AnimHash == 3730280596u)
            return thing;
    }

    return NULL;
}

MH_DefineFunc(SomePodFn0, 0xb0904, TOC0, void, RGame*);
MH_DefineFunc(SomePodFn1, 0xb526c, TOC0, void, RGame*);

bool SetInventoryPodController(void* inventory, CThing* global_thing, CResourceDescriptorBase const& plan)
{
    RLevel* level = gGame->Level;
    if (level == NULL) return false;
    CThing* thing = level->WorldThing;
    if (thing == NULL) return false;
    PWorld* world = thing->GetPWorld();
    if (world == NULL) return false;

    CThing* controller = world->FindThingByScriptName("pod_controller");
    if (controller == NULL)
    {
        DebugLog("Couldn't set pod controller mesh because thing does not exist in world!\n");
        return false;
    }

    PRenderMesh* controller_part = controller->GetPRenderMesh();
    if (controller_part == NULL)
    {
        DebugLog("Couldn't set pod controller mesh because thing does not have a PRenderMesh component!\n");
        return false;
    }

    PRenderMesh* plan_part = global_thing->GetPRenderMesh();
    if (plan_part == NULL)
    {
        DebugLog("Couldn't set pod controller mesh because global thing does not have a PRenderMesh component!\n");
        return false;
    }

    DebugLog("Attempting to set pod controller...\n");
    controller_part->Mesh = plan_part->Mesh;
    DebugLog("If we haven't crashed, controller mesh has been changed!\n");
    SomePodFn0(gGame);
    SomePodFn1(gGame);
    
    return true;
}

bool SetInventoryPodMesh(void* inventory, CThing* global_thing, CResourceDescriptorBase const& plan)
{
    RLevel* level = gGame->Level;
    if (level == NULL) return false;
    CThing* thing = level->WorldThing;
    if (thing == NULL) return false;
    PWorld* world = thing->GetPWorld();
    if (world == NULL) return false;

    CThing* pod = GetPodThing(world);
    if (pod == NULL)
    {
        DebugLog("Couldn't set pod mesh because thing does not exist in world!\n");
        return false;
    }

    PRenderMesh* pod_part = pod->GetPRenderMesh();
    if (pod_part == NULL)
    {
        DebugLog("Couldn't set pod mesh because thing does not have a PRenderMesh component!\n");
        return false;
    }

    PRenderMesh* plan_part = global_thing->GetPRenderMesh();
    if (plan_part == NULL)
    {
        DebugLog("Couldn't set pod mesh because global thing does not have a PRenderMesh component!\n");
        return false;
    }

    DebugLog("Attempting to set pod mesh...\n");
    pod_part->Mesh = plan_part->Mesh;
    DebugLog("If we haven't crashed, pod mesh has been changed!\n");
    SomePodFn0(gGame);
    SomePodFn1(gGame);
    
    return true;
}

extern "C" void  _setinventorypod_hook();
void InitPodStyles()
{
    MH_InitHook((void*)0x0002be3c, (void*)&GetPodThing);
    MH_Poke32(0x0038adc4, B(&_setinventorypod_hook, 0x0038adc4));
}