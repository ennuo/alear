#include "AlearHooks.h"
#include "AlearShared.h"

#include <cell/DebugLog.h>
#include <refcount.h>

#include <PoppetEnums.inl>
#include <PartPhysicsWorld.h>
#include <PartYellowHead.h>
#include <ResourceGame.h>
#include <ResourceGFXTexture.h>
#include <ResourceSystem.h>
#include <Poppet.h>

#include <thing.h>
#include <GFXApi.h>

enum ECursorSprite {
    CURSOR_UNLETHAL,
    CURSOR_ELECTRIC,
    CURSOR_FIRE,
    CURSOR_GAS,
    CURSOR_ICE,
    CURSOR_PLASMA,
    CURSOR_SPIKE,
    CURSOR_DROWNED,
    CURSOR_SMALL,
    CURSOR_VERTEX_EDIT,
    CURSOR_DOT_TO_DOT,
    CURSOR_STICKER_PICK,
    CURSOR_FLOOD_FILL,
    CURSOR_UV_EDIT,
    CURSOR_GLUE,
    CURSOR_STICKER_CUTTER,
    CURSOR_UNPHYSICS,
    CURSOR_EYEDROPPER,
    CURSOR_PAINTBRUSH,
    CURSOR_SPRAYCAN,
    CURSOR_STANDARD,
    CURSOR_MARQUEE,
    CURSOR_PHOTO_MARQUEE,
    CURSOR_CAPTURE_MARQUEE
};

bool IsLethalCursor(u32 sprite)
{
    return 
        sprite == CURSOR_UNLETHAL ||
        sprite == CURSOR_ELECTRIC || 
        sprite == CURSOR_FIRE ||
        sprite == CURSOR_GAS ||
        sprite == CURSOR_ICE ||
        sprite == CURSOR_PLASMA ||
        sprite == CURSOR_SPIKE || 
        sprite == CURSOR_DROWNED;
}

ECursorSprite GetCursorSprite(CPoppet* poppet)
{
    EPoppetMode mode = poppet->GetMode();
    EPoppetSubMode submode = poppet->GetSubMode();

    if (submode == SUBMODE_GAS_TWEAK) return CURSOR_GAS;
    if (mode != MODE_CURSOR) return CURSOR_STANDARD;

    switch (submode)
    {
        case SUBMODE_GRAB_PLAN: return CURSOR_CAPTURE_MARQUEE;
        case SUBMODE_GRAB_PHOTO: return CURSOR_PHOTO_MARQUEE;

        case SUBMODE_EDIT_VERTS: return CURSOR_VERTEX_EDIT;

        case SUBMODE_PICK_DECORATIONS: return CURSOR_STICKER_PICK;


        case SUBMODE_FLOOD_FILL:
            return CURSOR_FLOOD_FILL;

        case SUBMODE_UNPHYSICS:
            return CURSOR_UNPHYSICS;

        case SUBMODE_EYEDROPPER:
            return CURSOR_EYEDROPPER;
        
        case SUBMODE_DANGER:
        {
            switch (poppet->DangerMode)
            {
                case LETHAL_FIRE: return CURSOR_FIRE;
                case LETHAL_ELECTRIC: return CURSOR_ELECTRIC;
                case LETHAL_ICE: return CURSOR_ICE;
                case LETHAL_SPIKE: return CURSOR_SPIKE;

                case LETHAL_POISON_GAS:
                case LETHAL_POISON_GAS2: 
                case LETHAL_POISON_GAS3:
                case LETHAL_POISON_GAS4:
                case LETHAL_POISON_GAS5:
                case LETHAL_POISON_GAS6:
                    return CURSOR_GAS;

                case LETHAL_BULLET: return CURSOR_PLASMA;
                case LETHAL_DROWNED: return CURSOR_DROWNED;

                default: return CURSOR_UNLETHAL;
            }
        }

        default:
            return CURSOR_STANDARD;
    }

}

std::set<CPoppet*, std::less<CPoppet*>, STLBucketAlloc<CPoppet*> > gPoppetBloomHack;
void FixupCursorSpriteRect(CPoppet* poppet)
{
    bool bloom = gPoppetBloomHack.find(poppet) != gPoppetBloomHack.end();
    gPoppetBloomHack.insert(poppet);

    const u32 num_columns = 4;
    const u32 num_rows = 8;
    
    u32 icon_index = GetCursorSprite(poppet);
    f32 x = (f32)(icon_index % num_columns);
    f32 y = (f32)(icon_index / num_columns);

    v4 uv(            
        x / ((f32) num_columns),
        y / ((f32) num_rows),
        (x + 1.0f) / ((f32) num_columns),
        (y + 1.0f) / ((f32) num_rows)
    );

    v4 tl = v4(uv.getX(), uv.getY(), 0.0f, 0.0f);
    v4 tr = v4(uv.getZ(), uv.getY(), 0.0f, 0.0f);
    v4 br = v4(uv.getZ(), uv.getW(), 0.0f, 0.0f);
    v4 bl = v4(uv.getX(), uv.getW(), 0.0f, 0.0f);

    CTGVertex* vtx = NGfx::tgVtxCur - 4;

    *((v4*)vtx[0].tc0) = tl;
    *((v4*)vtx[1].tc0) = tr;
    *((v4*)vtx[2].tc0) = br;
    *((v4*)vtx[3].tc0) = bl;


    c32 color = poppet->PlayerThing->GetPYellowHead()->GetColour(PLAYER_COLOUR_PRIMARY);
    
    color = c32::White;
    if (bloom && icon_index != CURSOR_STANDARD)
    {
        color = HalfBright(color);
    }

    color = ReplaceA(color, 136);

    u32 bits = color.AsGPUCol();

    vtx[0].col = bits;
    vtx[1].col = bits;
    vtx[2].col = bits;
    vtx[3].col = bits;

    const v4 LETHAL_CURSOR_OFFSET(0.0f, 22.5f, 0.0f, 0.0f);
    const v4 FLOOD_FILL_CURSOR_OFFSET(15.0f, 17.5f, 0.0f, 0.0f);

    if (icon_index == CURSOR_FLOOD_FILL)
    {
        vtx[0].pos += FLOOD_FILL_CURSOR_OFFSET;
        vtx[1].pos += FLOOD_FILL_CURSOR_OFFSET;
        vtx[2].pos += FLOOD_FILL_CURSOR_OFFSET;
        vtx[3].pos += FLOOD_FILL_CURSOR_OFFSET;
    }
    else if (IsLethalCursor(icon_index))
    {
        vtx[0].pos += LETHAL_CURSOR_OFFSET;
        vtx[1].pos += LETHAL_CURSOR_OFFSET;
        vtx[2].pos += LETHAL_CURSOR_OFFSET;
        vtx[3].pos += LETHAL_CURSOR_OFFSET;
    }
}

void CPoppet::EyedropperPick(CThing* thing)
{
    CPoppet* poppet;
    CResourceDescriptor<RPlan> guid(33579);
    CResourceDescriptor<RPlan> body_guid(thing->PlanGUID);
    CResourceDescriptor<RPlan> gfx_guid(thing->GetPGeneratedMesh()->PlanGUID);
    if(thing->PlanGUID)
        poppet->FloodFillMaterialPlan = body_guid;
    else if(thing->GetPGeneratedMesh()->PlanGUID) 
        poppet->FloodFillMaterialPlan = gfx_guid;
    else
        poppet->FloodFillMaterialPlan = guid;
    poppet->FloodFillPhysicsMaterial = thing->GetPShape()->MMaterial;
    poppet->FloodFillSoundEnumOverride = thing->GetPShape()->SoundEnumOverride;
    poppet->FloodFillGfxMaterial = thing->GetPGeneratedMesh()->GfxMaterial;
    poppet->FloodFillBevel = thing->GetPGeneratedMesh()->Bevel;
    poppet->FloodFillBevelSize = thing->GetPShape()->BevelSize;
    poppet->PushMode(MODE_CURSOR, SUBMODE_FLOOD_FILL);
}

void CPoppet::EyedropperDrop(CThing* thing)
{

}

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
        case TOOL_SHAPE_ICE:
        {
            poppet->SendPoppetDangerMessage(LETHAL_ICE);
            break;
        }
        case TOOL_SHAPE_DROWNED:
        {
            poppet->SendPoppetDangerMessage(LETHAL_DROWNED);
            break;
        }
        case TOOL_SHAPE_SPIKE:
        {
            poppet->SendPoppetDangerMessage(LETHAL_SPIKE);
            break;
        }
        case TOOL_SHAPE_CRUSH:
        {
            poppet->SendPoppetDangerMessage(LETHAL_CRUSH);
            break;
        }
        case TOOL_SHAPE_PLASMA:
        {
            poppet->SendPoppetDangerMessage(LETHAL_BULLET);
            break;
        }
        case TOOL_EYEDROPPER:
        {
            //poppet->EyedropperPick();
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

void LoadCursorSprites()
{

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
    TABLE[TOOL_SHAPE_DROWNED] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_SPIKE] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_CRUSH] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_ICE] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_EYEDROPPER] = (u32)&_custom_tool_type_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0092ad18, (u32)TABLE);
}