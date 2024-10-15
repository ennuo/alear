#include "styles.h"
#include "Poppet.h"

#include "GFXApi.h"
#include "PartYellowHead.h"

#include "hook.h"

#include "alearconf.h"

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


void AlearInitStyles()
{
    MH_InitHook((void*)0x00344084, (void*)&OnFillPoppetBackground);
}