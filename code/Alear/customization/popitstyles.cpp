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
    float w = bubble_size.V[0];
    float h = bubble_size.V[1];
    
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

v2 CustomGetBubbleSize(CPoppet* poppet)
{

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

    PShape* shape = GetShapeTarget();
    if (shape != NULL) 
    {
        if (gLastNumVerts != shape->Polygon.size())
        {
            PGeneratedMesh* generated = GetMeshTarget();
            const CMesh* mesh = generated->SharedMesh;

            DebugLogChF(DC_DEFAULT, "Mesh refresh: CMesh has %d vertices\n", mesh->NumVerts);
            
            float* vertices = (float*)((char*)mesh->SourceGeometry.CachedAddress + mesh->SourceStreamOffsets[0]);
            for (int i = 0; i < mesh->NumVerts; ++i, vertices += 4)
            {
                DebugLogChF(DC_DEFAULT, "v %f %f %f\n", vertices[0], vertices[1], vertices[2]);
            }

            u16* indices = (u16*)(mesh->Indices.CachedAddress);
            DebugLogChF(DC_DEFAULT, "f %d %d %d\n", indices[0] + 1, indices[1] + 1, indices[2] + 1);
            for (int i = 3, j = 1; i < mesh->NumIndices; ++i, ++j)
            {
                if (indices[i] == 65535)
                {
                    if (i + 3 >= mesh->NumIndices) break;
                    DebugLogChF(DC_DEFAULT, "f %d %d %d\n", indices[i + 1] + 1, indices[i + 2] + 1, indices[i + 3] + 1);
                    i += 3;
                    j = 0;
                    continue;
                }

                if ((j & 1) != 0)
                    DebugLogChF(DC_DEFAULT, "f %d %d %d\n", indices[i - 2] + 1, indices[i] + 1, indices[i - 1] + 1);
                else
                    DebugLogChF(DC_DEFAULT, "f %d %d %d\n", indices[i - 2] + 1, indices[i - 1] + 1, indices[i] + 1);
            }

            gLastNumVerts = shape->Polygon.size();
        }

        bubble->SetBubbleShape(shape);
    }
    else bubble->SetBubbleRoundedRect(bubble_size.getX(), bubble_size.getY());
    

    // if (submode == SUBMODE_NONE) bubble->SetBubbleBalloon(bubble_size.getX(), bubble_size.getY(), 1.0f);
    // else bubble->SetBubbleRoundedRect(bubble_size.getX(), bubble_size.getY());

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