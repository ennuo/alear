#include "AlearHooks.h"
#include "AlearShared.h"

#include <cell/DebugLog.h>

#include <PoppetEnums.inl>
#include <PartPhysicsWorld.h>
#include <PartYellowHead.h>
#include <ResourceGame.h>
#include <Poppet.h>

#include <thing.h>

bool IsThingFady(CThing* thing)
{
    PWorld* world = gGame->GetWorld();
    if (thing == NULL || world == NULL) return false;
    if (thing->Stamping) return true;

    for (PYellowHead** it = world->ListPYellowHead.begin(); it != world->ListPYellowHead.end(); ++it)
    {
        PYellowHead* yellowhead = *it;
        CPoppet* poppet = yellowhead->Poppet;
        if (poppet == NULL || poppet->HiddenList.size() == 0) continue;
        for (CThingPtr* ptr = poppet->HiddenList.begin(); ptr != poppet->HiddenList.end(); ++ptr)
        {
            CThing* hidden = ptr->GetThing();
            if (hidden == thing) return true;
        }
    }

    PShape* shape = thing->GetPShape();
    if (shape == NULL || thing->GetPBody() == NULL) return false;
    if ((shape->InteractEditMode & 2) == 0) return false;


    if (!shape->CollidableGame)
    {
        for (PYellowHead** it = world->ListPYellowHead.begin(); it != world->ListPYellowHead.end(); ++it)
        {
            PYellowHead* yellowhead = *it;
            CPoppet* poppet = yellowhead->Poppet;
            if (poppet == NULL) continue;
            if (poppet->GetSubMode() == SUBMODE_UNPHYSICS)
                return true;
        }
    }

    return false;
}

bool SetUnphysics(CPoppet* poppet, CThing* thing)
{
    if (thing == NULL) return false;
    PShape* shape = thing->GetPShape();
    if (shape == NULL) return false;

    shape->SetCollidableGame(!shape->CollidableGame);
    shape->SetCollidablePoppet(true);
    
    return true;
}

void HandleCustomPoppetMessage(CPoppet* poppet, EPoppetMessageType msg)
{
    switch (msg)
    {
        case E_POPPET_UNPHYSICS_MESSAGE:
        {
            DebugLog("RECV: E_POPPET_UNPHYSICS_MESSAGE\n");
            poppet->PushMode(MODE_CURSOR, SUBMODE_UNPHYSICS);
            return;
        }
    }
}

void HandleCustomToolType(CPoppet* poppet, EToolType tool)
{
    switch (tool)
    {
        case TOOL_SHAPE_PLASMA:
        {
            poppet->SendPoppetDangerMessage(LETHAL_BULLET);
            break;
        }
        case TOOL_UNPHYSICS:
        {
            DebugLog("SEND: E_POPPET_UNPHYSICS_MESSAGE\n");
            poppet->SendPoppetMessage(E_POPPET_UNPHYSICS_MESSAGE);
            break;
        }
        case TOOL_POPIT_GRADIENT:
        {
            // poppet->SendPoppetMessage(E_POPPET_GRADIENT_MESSAGE)
            break;
        }
    }
}

void AttachCustomPoppetMessages()
{
    MH_Poke32(0x0034f978, 0x2b9d0000 + (E_POPPET_MESSAGE_TYPE_COUNT - 1));

    // Initialise the switch table with the offsets to the invalid resource type case
    const int SWITCH_LABEL = 0x0034f99c;
    const int NOP_LABEL = 0x0034fa40;
    const int LABEL_COUNT = 0x25;
    static s32 TABLE[E_POPPET_MESSAGE_TYPE_COUNT];
    for (int i = 0; i < E_POPPET_MESSAGE_TYPE_COUNT; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    TABLE[E_POPPET_UNPHYSICS_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0092afb4, (u32)TABLE);
}

void AttachCustomToolTypes()
{
    MH_Poke32(0x003466cc, 0x2b8a0000 + (NUM_TOOL_TYPES - 1));

    // Initialise the switch table with the offsets to the invalid resource type case
    const int SWITCH_LABEL = 0x0034675c;
    const int NOP_LABEL = 0x003466d4;
    const int LABEL_COUNT = 0x1e;
    static s32 TABLE[NUM_TOOL_TYPES];
    for (int i = 0; i < NUM_TOOL_TYPES; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    TABLE[TOOL_UNPHYSICS] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_PLASMA] = (u32)&_custom_tool_type_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0092ad18, (u32)TABLE);
}