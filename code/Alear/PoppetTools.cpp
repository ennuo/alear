#include "AlearHooks.h"
#include "AlearShared.h"
#include "AlearConfig.h"

#include <GuidHash.h>

#include <cell/DebugLog.h>
#include <cell/fs/cell_fs_file_api.h>
#include <refcount.h>

#include <PoppetEnums.inl>
#include <PartPhysicsWorld.h>
#include <PartYellowHead.h>
#include <ResourceGame.h>
#include <ResourceGFXTexture.h>
#include <ResourceGFXMesh.h>
#include <ResourcePlan.h>
#include <ResourceBevel.h>
#include <ResourceMaterial.h>
#include <ResourceSystem.h>
#include <Poppet.h>
#include <fmod.h>

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
    CURSOR_VERTEX_EDIT,
    CURSOR_SLICE_N_DICE,
    CURSOR_DOT_TO_DOT,
    CURSOR_STICKER_PICK,
    CURSOR_FLOOD_FILL,
    CURSOR_UV_EDIT,
    CURSOR_GLUE,
    CURSOR_STICKER_CUTTER,
    CURSOR_STICKER_SCRUBBER,
    CURSOR_EYEDROPPER,
    CURSOR_PAINTBRUSH,
    CURSOR_SPRAYCAN,
    CURSOR_STANDARD,
    CURSOR_STANDARD_MARQUEE,
    CURSOR_PHOTO_MARQUEE,
    CURSOR_CAPTURE_MARQUEE,
    CURSOR_CROSSHAIRS,
    CURSOR_RESIZE,
    CURSOR_ROTATE,
    CURSOR_28,
    CURSOR_UNPHYSICS,
    CURSOR_MESH_CAPTURE,
    CURSOR_31,
    CURSOR_32
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
    if (mode != MODE_CURSOR) return CURSOR_STANDARD;
    if(!gUseCustomCursors)
    {
        switch (submode)
        {
            case SUBMODE_GRAB_PHOTO:
            case SUBMODE_GRAB_PLAN:
                return CURSOR_STANDARD_MARQUEE;
            default:
                return CURSOR_STANDARD;
        }
    }

    if (submode == SUBMODE_GAS_TWEAK) return CURSOR_GAS;
    if (submode == SUBMODE_PLASMA_TWEAK) return CURSOR_PLASMA;
    switch (submode)
    {
        case SUBMODE_RESIZE: return CURSOR_RESIZE;
        case SUBMODE_ROTATE: return CURSOR_ROTATE;
        case SUBMODE_GRAB_PLAN: 
        case SUBMODE_GRAB_PLAN_MARQUEE:
            return CURSOR_CAPTURE_MARQUEE;
        case SUBMODE_GRAB_PHOTO: return CURSOR_PHOTO_MARQUEE;
        case SUBMODE_EDIT_VERTS: return CURSOR_VERTEX_EDIT;
        case SUBMODE_SLICE_N_DICE:
        case SUBMODE_OBJECT_EDIT_SLICE_N_DICE:
            return CURSOR_SLICE_N_DICE;
        case SUBMODE_STICKER_SCRUBBER: return CURSOR_STICKER_SCRUBBER;
        case SUBMODE_STICKER_CUTTER: return CURSOR_STICKER_CUTTER;
        case SUBMODE_PICK_DECORATIONS: return CURSOR_STICKER_PICK;
        case SUBMODE_FLOOD_FILL: return CURSOR_FLOOD_FILL;
        case SUBMODE_ADVANCED_GLUE: return CURSOR_GLUE;
        case SUBMODE_EDIT_UVS: return CURSOR_UV_EDIT;
        case SUBMODE_UNPHYSICS: return CURSOR_UNPHYSICS;
        case SUBMODE_EYEDROPPER: return CURSOR_EYEDROPPER;
        case SUBMODE_DOT_TO_DOT: return CURSOR_DOT_TO_DOT;
        case SUBMODE_PAINT_SCRIBBLE:
        case SUBMODE_PAINT_LINES:
            return CURSOR_PAINTBRUSH;
        case SUBMODE_PAINT_SPRAY: return CURSOR_SPRAYCAN;
        case SUBMODE_MESH_CAPTURE: return CURSOR_MESH_CAPTURE;
        
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
        
        case SUBMODE_OBJECT_EDIT_BASIC:
        {
            
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
    
    if(!gColorCustomCursors) { color = c32::White; }
    if (bloom && !(icon_index == CURSOR_STANDARD || icon_index == CURSOR_STANDARD_MARQUEE))
    {
        color = HalfBright(color);
    }

    if(!gColorCustomCursors) { color = ReplaceA(color, 136); }

    u32 bits = color.AsGPUCol();

    vtx[0].col = bits;
    vtx[1].col = bits;
    vtx[2].col = bits;
    vtx[3].col = bits;

    const v4 STANDARD_CURSOR_SCALE(1.0f, 1.0f, 1.0f, 1.0f);

    const v4 LETHAL_CURSOR_OFFSET(0.0f, 22.5f, 0.0f, 0.0f);
    const v4 FLOOD_FILL_CURSOR_OFFSET(15.0f, 17.5f, 0.0f, 0.0f);
    const v4 EYEDROPPER_CURSOR_OFFSET(25.0f, 25.0f, 0.0f, 0.0f);
    const v4 STICKER_SCRUBBER_CURSOR_OFFSET(10.0f, 5.0f, 0.0f, 0.0f);

    if (icon_index == CURSOR_FLOOD_FILL)
    {
        vtx[0].pos += FLOOD_FILL_CURSOR_OFFSET;
        vtx[1].pos += FLOOD_FILL_CURSOR_OFFSET;
        vtx[2].pos += FLOOD_FILL_CURSOR_OFFSET;
        vtx[3].pos += FLOOD_FILL_CURSOR_OFFSET;
    }
    if (icon_index == CURSOR_EYEDROPPER)
    {
        vtx[0].pos += EYEDROPPER_CURSOR_OFFSET;
        vtx[1].pos += EYEDROPPER_CURSOR_OFFSET;
        vtx[2].pos += EYEDROPPER_CURSOR_OFFSET;
        vtx[3].pos += EYEDROPPER_CURSOR_OFFSET;
    }
    if (icon_index == CURSOR_STICKER_SCRUBBER)
    {
        vtx[0].pos += STICKER_SCRUBBER_CURSOR_OFFSET;
        vtx[1].pos += STICKER_SCRUBBER_CURSOR_OFFSET;
        vtx[2].pos += STICKER_SCRUBBER_CURSOR_OFFSET;
        vtx[3].pos += STICKER_SCRUBBER_CURSOR_OFFSET;
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
    //PushMode(MODE_NORMAL, SUBMODE_CHOOSE_MATERIAL);
    PushMode(MODE_CURSOR, SUBMODE_FLOOD_FILL);
    CResourceDescriptor<RPlan> body_guid(thing->PlanGUID);
    CResourceDescriptor<RPlan> gfx_guid(thing->GetPGeneratedMesh()->PlanGUID);
    PGeneratedMesh* generated_mesh = thing->GetPGeneratedMesh();
    PShape* shape = thing->GetPShape();
    if(thing->GetPGeneratedMesh()->PlanGUID) 
    {
        FloodFillMaterialPlan = gfx_guid;
    }
    else if(thing->PlanGUID)
    {
        FloodFillMaterialPlan = body_guid;
    }
    CP<RMaterial>& physics_material = shape->MMaterial;
    FloodFillPhysicsMaterial = physics_material;
    CP<RGfxMaterial>& gmat = generated_mesh->GfxMaterial;
    FloodFillGfxMaterial = gmat;
    CP<RBevel>& bevel = thing->GetPGeneratedMesh()->Bevel;
    FloodFillBevel = bevel;
    float bevel_size = shape->BevelSize;
    FloodFillBevelSize = bevel_size;
    u32 sound_enum = shape->SoundEnumOverride;
    FloodFillSoundEnumOverride = sound_enum;
}

void CPoppet::EyedropperPickMesh(CThing* thing)
{
    //PushMode(MODE_NORMAL, SUBMODE_CHOOSE_MATERIAL);
    PushMode(MODE_CURSOR, SUBMODE_FLOOD_FILL);
    CResourceDescriptor<RPlan> body_guid(thing->PlanGUID);
    PRenderMesh* render_mesh = thing->GetPRenderMesh();
    if (render_mesh != NULL)
    {
        CP<RMesh>& mesh = render_mesh->Mesh;
        if (!mesh || !mesh->IsLoaded()) return;
        CVector<CPrimitive>& primitives = mesh->mesh.Primitives;
        for (CPrimitive* it = primitives.begin(); it != primitives.end(); ++it)
        {
            CPrimitive& primitive = *it;
            CP<RGfxMaterial>& gmat = primitive.Material;
            if (!gmat || !gmat->IsLoaded()) return;
            FloodFillGfxMaterial = gmat;
        }
    }
    if(thing->PlanGUID)
    {
        FloodFillMaterialPlan = body_guid;
    }
    CP<RBevel> bevel;
    FloodFillBevel = bevel;
    PShape* shape = thing->GetPShape();
    if(shape)
    {
        CP<RMaterial>& physics_material = shape->MMaterial;
        FloodFillPhysicsMaterial = physics_material;
        float bevel_size = shape->BevelSize;
        FloodFillBevelSize = bevel_size;
        u32 sound_enum = shape->SoundEnumOverride;
        FloodFillSoundEnumOverride = sound_enum;
    }
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

bool DumpMeshToFile(CPoppet* poppet, CThing* thing)
{
    PGeneratedMesh* generated = thing->GetPGeneratedMesh();
    PPos* part_pos = thing->GetPPos();
    if (generated == NULL || part_pos == NULL) return false;

    const CMesh* mesh = generated->SharedMesh;

    char path[CELL_FS_MAX_FS_FILE_NAME_LENGTH];
    sprintf(path, "output/meshgen/%d_glb", thing->UID);
    CFilePath fp(FPR_GAMEDATA, path);

    FileHandle fd;
    FileOpen(fp, &fd, OPEN_WRITE);

    MMString<char> gltf;
    gltf.reserve(10000); // reserve a reasonable amount of space for all the strcat bullshit

    float* vertices = (float*)((char*)mesh->SourceGeometry.GetCachedAddress() + mesh->SourceStreamOffsets[0]);
    float* uv = ((float*)mesh->AttributeData.GetCachedAddress());

    u16* indices = (u16*)(mesh->Indices.GetCachedAddress());
    CRawVector<u16> triangles((mesh->NumVerts - 2) * 3);
    triangles.push_back(indices[0]);
    triangles.push_back(indices[1]);
    triangles.push_back(indices[2]);
    for (int i = 3, j = 1; i < mesh->NumIndices; ++i, ++j)
    {
        if (indices[i] == 65535)
        {
            if (i + 3 >= mesh->NumIndices) break;

            triangles.push_back(indices[i + 1]);
            triangles.push_back(indices[i + 2]);
            triangles.push_back(indices[i + 3]);

            i += 3;
            j = 0;
            continue;
        }

        if ((j & 1) != 0)
        {
            triangles.push_back(indices[i - 2]);
            triangles.push_back(indices[i]);
            triangles.push_back(indices[i - 1]);
        }
        else
        {
            triangles.push_back(indices[i - 2]);
            triangles.push_back(indices[i - 1]);
            triangles.push_back(indices[i]);
        }
    }

    FileClose(&fd);
    return true;
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

bool ClearStickers(CPoppet* poppet, CThing* thing)
{
    if (thing == NULL) return false;
    PStickers* stickers = thing->GetPStickers();
    if (stickers == NULL) return true;
    thing->RemovePart(PART_TYPE_STICKERS);
    
    return true;
}

bool EyedropperPickObject(CPoppet* poppet, CThing* thing)
{
    if (thing == NULL) return false;
    PRenderMesh* render_mesh = thing->GetPRenderMesh();
    v2 pos3d = *(v2*)(((char*)poppet) + 0x180);
    if(gAllowEyedroppingMeshes && render_mesh != NULL) {
        poppet->EyedropperPickMesh(thing); 
    }
    else 
    {
        PShape* shape = thing->GetPShape();
        if (shape == NULL) return false;
        PGeneratedMesh* generated_mesh = thing->GetPGeneratedMesh();
        if (generated_mesh == NULL) return false;
        poppet->EyedropperPick(thing);
        //CAudio::PlaySample(CAudio::gSFX, "character/accessories/smelly_stuff/select", thing, -10000.0f, -10000.0f);
    }
    
    return true;
}

// Marquee selectable
bool ToolHasMarquee(CPoppet* poppet)
{
    EPoppetSubMode submode = poppet->GetSubMode();
    switch (submode)
    {
        case SUBMODE_NONE:
        case SUBMODE_DANGER:
        case SUBMODE_FLOOD_FILL:
        case SUBMODE_STICKER_CUTTER:
        case SUBMODE_STICKER_SCRUBBER:
        case SUBMODE_UNPHYSICS:
        case SUBMODE_ADVANCED_GLUE:
        case SUBMODE_GRAB_PLAN_MARQUEE:
        case SUBMODE_MESH_CAPTURE:
            break;
        default:
            return false;
    }
    return true;
}

// Marquee action
bool HandleMarqueeAction(CPoppet* poppet)
{
    poppet->Backup();
    CVector<CThingPtr>& things = *(CVector<CThingPtr>*)(((char*)poppet) + 0x170); 
    EPoppetSubMode submode = poppet->GetSubMode();
    v2 pos3d = *(v2*)(((char*)poppet) + 0x180);
    switch (submode)
    {
        case SUBMODE_FLOOD_FILL:
            for (int i = 0; i < things.size(); ++i)
            {
                if(poppet->FloodFill(things[i])) { CAudio::PlaySample(CAudio::gSFX, "poppet/floodfill", things[i], -10000.0f, -10000.0f); }
            }
            break;
        case SUBMODE_DANGER:
            for (int i = 0; i < things.size(); ++i)
            {
                poppet->SetDangerType(things[i]);
            }
            break;
        case SUBMODE_STICKER_SCRUBBER:
            for (int i = 0; i < things.size(); ++i)
            {
                ClearStickers(poppet, things[i]);
            }
            break;
        case SUBMODE_UNPHYSICS:
            for (int i = 0; i < things.size(); ++i)
            {
                SetUnphysics(poppet, things[i]);
            }
            break;
        case SUBMODE_MESH_CAPTURE:
            for (int i = 0; i < things.size(); ++i)
            {
                DumpMeshToFile(poppet, things[i]);
            }
            break;
        default:
            return false;
    }
    return true;
}

// Pick object action
/*
bool HandlePickObjectAction(CPoppet* poppet, CThing* thing)
{
    EPoppetSubMode submode = poppet->GetSubMode();
    switch (submode)
    {
        case SUBMODE_DANGER:
            poppet->SetDangerType(thing);
            break;
        case SUBMODE_EYEDROPPER:
            EyedropperPickObject(poppet, thing);
            break;
        case SUBMODE_STICKER_CUTTER:
            break;
        case SUBMODE_STICKER_SCRUBBER:
            break;
        case SUBMODE_SLICE_N_DICE:
            break;
        case SUBMODE_UNPHYSICS:
            SetUnphysics(poppet, thing);
            break;
        case SUBMODE_ADVANCED_GLUE:
            break;
        case SUBMODE_EDIT_UVS:
            break;
        case SUBMODE_GRAB_PLAN_MARQUEE:
            break;
        default:
            return false;
    }
    return true;
}
*/

void HandleCustomPoppetMessage(CPoppet* poppet, EPoppetMessageType msg)
{
    switch (msg)
    {
        case E_POPPET_PLASMA_TWEAK_MESSAGE:
        {
            poppet->PushMode(MODE_TWEAK, SUBMODE_PLASMA_TWEAK);
            break;
        }
        case E_POPPET_UNPHYSICS_MESSAGE:
        {
            DebugLog("RECV: E_POPPET_UNPHYSICS_MESSAGE\n");
            poppet->PushMode(MODE_CURSOR, SUBMODE_UNPHYSICS);
            return;
        }
        case E_POPPET_EYEDROPPER_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_EYEDROPPER);
            return;
        }
        case E_POPPET_DOT_TO_DOT_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_DOT_TO_DOT);
            return;
        }
        case E_POPPET_STICKER_SCRUBBER_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_STICKER_SCRUBBER);
            break;
        }
        case E_POPPET_STICKER_CUTTER_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_STICKER_CUTTER);
            break;
        }
        case E_POPPET_SLICE_N_DICE_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_SLICE_N_DICE);
            break;
        }
        case E_POPPET_UV_EDIT_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_EDIT_UVS);
            break;
        }
        case E_POPPET_ADVANCED_GLUE_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_ADVANCED_GLUE);
            break;
        }
        case E_POPPET_PLAN_MARQUEE_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_GRAB_PLAN_MARQUEE);
            break;
        }
        case E_POPPET_RANDOM_STICKER_MESSAGE:
        {
            break;
        }
        case E_POPPET_RANDOM_DECORATION_MESSAGE:
        {
            break;
        }
        case E_POPPET_RANDOM_MATERIAL_MESSAGE:
        {
            break;
        }
        case E_POPPET_RANDOM_OBJECT_MESSAGE:
        {
            break;
        }
        case E_POPPET_MESH_CAPTURE_MESSAGE:
        {
            poppet->PushMode(MODE_CURSOR, SUBMODE_MESH_CAPTURE);
            break;
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
        case TOOL_UNPHYSICS:
        {
            DebugLog("SEND: E_POPPET_UNPHYSICS_MESSAGE\n");
            poppet->SendPoppetMessage(E_POPPET_UNPHYSICS_MESSAGE);
            break;
        }
        case TOOL_EYEDROPPER:
        {
            poppet->SendPoppetMessage(E_POPPET_EYEDROPPER_MESSAGE);
            break;
        }
        case TOOL_DOT_TO_DOT:
        {
            poppet->SendPoppetMessage(E_POPPET_DOT_TO_DOT_MESSAGE);
            break;
        }
        case TOOL_STICKER_WASH:
        {
            poppet->SendPoppetMessage(E_POPPET_STICKER_SCRUBBER_MESSAGE);
            break;
        }
        case TOOL_STICKER_CUTTER:
        {
            poppet->SendPoppetMessage(E_POPPET_STICKER_CUTTER_MESSAGE);
            break;
        }
        case TOOL_SLICE_N_DICE:
        {
            poppet->SendPoppetMessage(E_POPPET_SLICE_N_DICE_MESSAGE);
            break;
        }
        case TOOL_UV_EDIT:
        {
            poppet->SendPoppetMessage(E_POPPET_UV_EDIT_MESSAGE);
            break;
        }
        case TOOL_GLUE:
        {
            poppet->SendPoppetMessage(E_POPPET_ADVANCED_GLUE_MESSAGE);
            break;
        }
        case TOOL_CURSOR:
        {
            poppet->SendPoppetMessage(E_POPPET_CURSOR_MESSAGE);
            break;
        }

        case TOOL_RANDOM_STICKER:
        {
            poppet->SendPoppetMessage(E_POPPET_RANDOM_STICKER_MESSAGE);
            break;
        }
        case TOOL_RANDOM_DECORATION:
        {
            poppet->SendPoppetMessage(E_POPPET_RANDOM_DECORATION_MESSAGE);
            break;
        }
        case TOOL_RANDOM_MATERIAL:
        {
            poppet->SendPoppetMessage(E_POPPET_RANDOM_MATERIAL_MESSAGE);
            break;
        }
        case TOOL_RANDOM_OBJECT:
        {
            poppet->SendPoppetMessage(E_POPPET_RANDOM_OBJECT_MESSAGE);
            break;
        }

        case TOOL_MESH_CAPTURE:
        {
            poppet->SendPoppetMessage(E_POPPET_MESH_CAPTURE_MESSAGE);
            break;
        }
        case TOOL_EXPLOSION:
        {
            poppet->SendPoppetMessage(E_POPPET_EXPLOSION_MESSAGE);
            break;
        }
        case TOOL_GENEALOGY:
        {
            poppet->SendPoppetMessage(E_POPPET_GENEALOGY_MESSAGE);
            break;
        }
        case TOOL_POPIT_GRADIENT:
        {
            //poppet->SendPoppetMessage(E_POPPET_GRADIENT_MESSAGE);
            break;
        }
    }
}

bool HandleToolPickSound(CPoppet* poppet, CThing* thing)
{
    EPoppetSubMode submode = poppet->GetSubMode();
    switch (submode)
    {
        case SUBMODE_UNPHYSICS:
        case SUBMODE_STICKER_CUTTER:
        case SUBMODE_MESH_CAPTURE:
            CAudio::PlaySample(CAudio::gSFX, "poppet/placeglue", thing, -10000.0f, -10000.0f);
            break;
        default:
            return false;
    }
    return true;
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
    TABLE[E_POPPET_EYEDROPPER_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_DOT_TO_DOT_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_STICKER_SCRUBBER_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_STICKER_CUTTER_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_SLICE_N_DICE_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_UV_EDIT_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_ADVANCED_GLUE_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_PLAN_MARQUEE_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_PLASMA_TWEAK_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    
    TABLE[E_POPPET_RANDOM_STICKER_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_RANDOM_DECORATION_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_RANDOM_MATERIAL_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_RANDOM_OBJECT_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    
    TABLE[E_POPPET_MESH_CAPTURE_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_EXPLOSION_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_GENEALOGY_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;
    TABLE[E_POPPET_GRADIENT_MESSAGE] = (u32)&_custom_poppet_message_hook - (u32)TABLE;

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

    TABLE[TOOL_CURSOR] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_UNPHYSICS] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_ICE] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_PLASMA] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_SPIKE] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_DROWNED] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SHAPE_CRUSH] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_EYEDROPPER] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_DOT_TO_DOT] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_STICKER_WASH] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_STICKER_CUTTER] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_SLICE_N_DICE] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_UV_EDIT] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_GLUE] = (u32)&_custom_tool_type_hook - (u32)TABLE;

    TABLE[TOOL_RANDOM_STICKER] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_RANDOM_DECORATION] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_RANDOM_MATERIAL] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_RANDOM_OBJECT] = (u32)&_custom_tool_type_hook - (u32)TABLE;

    TABLE[TOOL_MESH_CAPTURE] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_EXPLOSION] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_GENEALOGY] = (u32)&_custom_tool_type_hook - (u32)TABLE;
    TABLE[TOOL_POPIT_GRADIENT] = (u32)&_custom_tool_type_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0092ad18, (u32)TABLE);
}