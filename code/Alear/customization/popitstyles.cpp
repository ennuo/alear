#include "customization/popitstyles.h"
#include "alearconf.h"

#include <hook.h>

#include <GFXApi.h>
#include <Poppet.h>
#include <PoppetBubble.h>
#include <PartYellowHead.h>

#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <thing.h>

#include <gooey/GooeyNodeManager.h>
#include <cell/DebugLog.h>

void OnFillInfoBubbleBackground(CInfoBubble* bubble, CGooeyNodeManager* manager)
{
    u32 col0 = ReplaceA(manager->PrimaryColour, 0xcc).AsGPUCol();
    u32 col1 = ReplaceA(manager->HighlightColour, 0xcc).AsGPUCol();

    v2 max = bubble->BubbleMax;
    v2 min = bubble->BubbleMin; 
    v2 size = (max - min) * 0.25f;

    v4 tl = v4(min.getX() - size.getX(), min.getY() - size.getY(), 0.0f, 1.0f);
    v4 tr = v4(max.getX() + size.getX(), min.getY() - size.getY(), 0.0f, 1.0f);
    v4 br = v4(max.getX() + size.getX(), max.getY() + size.getY(), 0.0f, 1.0f);
    v4 bl = v4(min.getX() - size.getX(), max.getY() + size.getY(), 0.0f, 1.0f);

    NGfx::tgStart(4, 0x30);

    NGfx::tgAddVertex(tl, col0, 0.0f, 0.0f, 0.0f, 0.0f);
    NGfx::tgAddVertex(tr, col0, 0.0f, 0.0f, 0.0f, 0.0f);
    NGfx::tgAddVertex(br, col1, 0.0f, 0.0f, 0.0f, 0.0f);
    NGfx::tgAddVertex(bl, col1, 0.0f, 0.0f, 0.0f, 0.0f);

    NGfx::tgDraw(CELL_GCM_PRIMITIVE_QUADS, NULL, true, 1, 0, true, v4(0.0f, 0.0f, 0.0f, 1.0f));
}

void OnFillPoppetBackground(CPoppet* poppet, float alpha)
{
    CThing* player = poppet->PlayerThing;
    if (player == NULL) return;
    PYellowHead* yellowhead = player->GetPYellowHead();
    if (yellowhead == NULL) return;

    c32 primary = yellowhead->GetColour(PLAYER_COLOUR_PRIMARY);
    c32 secondary = yellowhead->GetColour(PLAYER_COLOUR_SECONDARY);
    c32 tertiary = yellowhead->GetColour(PLAYER_COLOUR_TERTIARY);

    v2 bubble_size = poppet->GetBubbleSize();
    float w = bubble_size.getX();
    float h = bubble_size.getY();
    
    u32 a = (u32)(alpha * 255.0f);

    u32 col0, col1;
    if (gUsePopitGradients)
    {
        col0 = (secondary.Bits & 0xffffff) | (a << 0x18);
        col1 = (primary.Bits & 0xffffff) | (a << 0x18);
    }
    else
    {
        col0 = (primary.Bits & 0xffffff) | (a << 0x18);
        col1 = (((((primary.Bits >> 1) & 0x7f7f7f7f) >> 1) & 0x7f7f7f7f) & 0xffffff) | (a << 0x18);
    }

    col0 = (col0 << 8) | (col0 >> 0x18);
    col1 = (col1 << 8) | (col1 >> 0x18);

    float u = (w * 0.25f) / 96.0f;
    float v = (h * 0.25f) / 96.0f;

    v4 tl(-(w * 0.25f), -(h * 0.25f), 0.0f, 1.0f);
    v4 tr((w * 0.25f) + w, -(h * 0.25f), 0.0f, 1.0f);
    v4 br((w * 0.25f) + w, (h * 0.25f) + h, 0.0f, 1.0f);
    v4 bl(-(w * 0.25f), (h * 0.25f) + h, 0.0f, 1.0f);

    NGfx::tgStart(4, 0x30);

    NGfx::tgAddVertex(tl, col0, 0.0f, 0.0f, 0.0f, 0.0f);
    NGfx::tgAddVertex(tr, col0, u, 0.0f, 0.0f, 0.0f);
    NGfx::tgAddVertex(br, col1, u, v, 0.0f, 0.0f);
    NGfx::tgAddVertex(bl, col1, 0.0f, v, 0.0f, 0.0f);

    NGfx::tgDraw(CELL_GCM_PRIMITIVE_QUADS, NULL, true, 1, 0, true, v4(0.0f, 0.0f, 0.0f, 1.0f));
}

const float REMOTE_HEIGHT = 200.0f;
const float REMOTE_WIDTH = 200.0f;
const float MAIN_EDIT_HEIGHT = 544.0f;
const float MAIN_EDIT_WIDTH = 512.0f;
const float MAIN_PLAY_HEIGHT = 450.0f;
const float MAIN_PLAY_WIDTH = 320.0f;
const float POPPET_SINGLE_PLAYER_WIDTH = 928.0f;
const float TWEAK_MENU_WIDTH = 592.0f;
const float POPPET_DEFAULT_WIDTH = 512.0f;
const float DOCK_HEIGHT_2 = 1900.0f;
const float DOCK_HEIGHT_1 = 950.0f;
const float SINGLE_PLAYER_HEIGHT = 1024.0f;
const float DEFAULT_HEIGHT = 1024.0f;



v2 CustomGetBubbleSize(CPoppet* poppet)
{
    EPoppetMode mode = poppet->GetMode();
    EPoppetSubMode submode = poppet->GetSubMode();

    if (poppet->IsDocked())
    {
        float width = POPPET_DEFAULT_WIDTH;
        float height = DOCK_HEIGHT_1;

        v4 dock_pos = poppet->GetDockPos();
        if (dock_pos.getZ() > 1.0f) width = POPPET_SINGLE_PLAYER_WIDTH;
        else if (mode == MODE_TWEAK) width = TWEAK_MENU_WIDTH;

        if (dock_pos.getW() > 1.0f) height = DOCK_HEIGHT_2;

        return v2(width, height);
    }

    if (submode == SUBMODE_REMOTE) return v2(REMOTE_WIDTH, REMOTE_HEIGHT);
    if (mode == MODE_MENU)
    {
        PYellowHead* yellowhead = poppet->PlayerThing->GetPYellowHead();
        if (yellowhead != NULL && yellowhead->SuicideHoldFrameCount != 0)
            return v2(250.0f, 250.0f);
        
        if (submode == SUBMODE_EMOTES)
            return v2(MAIN_EDIT_WIDTH, MAIN_EDIT_HEIGHT);

        if (submode != SUBMODE_NONE)
            return v2(POPPET_SINGLE_PLAYER_WIDTH, SINGLE_PLAYER_HEIGHT);
        
        if (gGame->EditMode)
            return v2(MAIN_EDIT_WIDTH, MAIN_EDIT_HEIGHT);

        return v2(MAIN_PLAY_WIDTH, MAIN_PLAY_HEIGHT);
    }

    if (mode == MODE_CURSOR)
    {
        if (submode == SUBMODE_GRAB_PLAN || submode == SUBMODE_GRAB_PHOTO)
            return poppet->Inventory.SelectBoxBounds;
        return v2(10.0f, 10.0f);
    }

    if (mode != MODE_TWEAK) return v2(0.0f);

    return v2(TWEAK_MENU_WIDTH, MAIN_EDIT_HEIGHT);
}

PGeneratedMesh* GetMeshTarget()
{
    RLevel* level = gGame->Level;
    if (level == NULL) return NULL;
    CThing* thing = level->WorldThing;
    if (thing == NULL) return NULL;
    PWorld* world = thing->GetPWorld();
    if (world == NULL) return NULL;

    CThing* target_thing = world->GetThingByUID(5588);
    if (target_thing != NULL)
        return target_thing->GetPGeneratedMesh();

    return NULL;
}

PShape* GetShapeTarget()
{
    RLevel* level = gGame->Level;
    if (level == NULL) return NULL;
    CThing* thing = level->WorldThing;
    if (thing == NULL) return NULL;
    PWorld* world = thing->GetPWorld();
    if (world == NULL) return NULL;

    CThing* target_thing = world->GetThingByUID(5588);
    if (target_thing != NULL)
        return target_thing->GetPShape();

    return NULL;
}


int gLastNumVerts = 0;

void CustomUpdateShape(CPoppetBubble* bubble, bool in_use)
{
    CPoppet* poppet = bubble->GetParent();

    v2 bubble_size = poppet->GetBubbleSize();
    EPoppetMode mode = poppet->GetMode();
    EPoppetSubMode submode = poppet->GetSubMode();

    bubble->SetBubbleRoundedRect(bubble_size.getX(), bubble_size.getY());

    if (submode == SUBMODE_EMOTES) bubble->SetBubbleCircle(MAX(bubble_size.getX(), bubble_size.getY()) / 2.0f);
    else bubble->SetBubbleRoundedRect(bubble_size.getX(), bubble_size.getY());

    u32 id;
    switch (submode)
    {
        case SUBMODE_INVENTORY: id = 0x3ab; break;
        case SUBMODE_EYETOY: id = 0x3af; break;
        case SUBMODE_ORGANISE_PAGE: id = 0x3a3; break;
        case SUBMODE_DEBUG_MENU: id = 0x3a7; break;
        case SUBMODE_REMOTE: id = 0x3b3; break;
        default: id = 0x3b7; break;
    }

    bubble->SetBubbleModeID(id);
}