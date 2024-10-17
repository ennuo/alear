#include "podstyles.h"


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

const u32 gUserObjectMask = 0x40100480;

bool CustomItemMatch(CInventoryView* view, CInventoryItem* item, NetworkPlayerID* owner)
{
    CGUID item_guid = item->Plan.GetGUID();

    u32 item_type = item->Details.Type;
    u32 view_type = view->Descriptor.Type;

    u32 item_subtype = item->Details.SubType;
    u32 view_subtype = view->Descriptor.SubType;

    if (view->HeartedOnly && (item->Flags & E_IIF_HEARTED) == 0) return false;
    if (item_guid == 0x12981 || item_guid == 0x15351) return false;
    if ((item_type & view_type) == 0) return false;

    if ((view_type & E_TYPE_USER_POD) != 0)
    {
        if ((item_type & E_TYPE_POD_TOOL) != 0) return true;

        if ((item_subtype & E_SUBTYPE_POD_CONTROLLER) != 0 || (item_subtype & E_SUBTYPE_POD_MESH) != 0)
            return (item_subtype & view_subtype) != 0;
        
        return (view_subtype & E_SUBTYPE_POD_CONTROLLER) == 0 && (view_subtype & E_SUBTYPE_POD_MESH) == 0;
    }

    if ((item_type & gUserObjectMask) == 0)
    {
        if (item_subtype != 0)
            return (item_subtype & view_subtype) != 0;

        return true;
    }
    
    bool owns = item->Details.ToolType == TOOL_NOT_A_TOOL 
                ? item->Details.IsCreatedBy(owner)
                : (item_subtype & E_SUBTYPE_MADE_BY_OTHERS) == 0;
    
    if ((view_subtype & E_SUBTYPE_MADE_BY_ME) != 0 && owns)
        return true;
    
    if ((view_subtype & E_SUBTYPE_MADE_BY_OTHERS) != 0)
        return !owns;

    return false;
}

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
    MH_InitHook((void*)0x002eeb90, (void*)&CustomItemMatch);
    MH_InitHook((void*)0x0002be3c, (void*)&GetPodThing);
    MH_Poke32(0x0038adc4, B(&_setinventorypod_hook, 0x0038adc4));
}