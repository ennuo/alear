#include "AlearHooks.h"
#include "InventoryItemRequest.h"
#include "AlearShared.h"
#include "AlearStartMenu.h"
#include "AlearDebugCamera.h"
#include "AlearConfig.h"
#include "PinSystem.h"
#include "OutfitSystem.h"

#include "customization/SlapStyles.h"
#include "customization/Emotes.h"

#include <map>

#include <cell/fs/cell_fs_file_api.h>
#include <cell/gcm.h>

#include <hook.h>
#include <json.h>
#include <printf.h>
#include <gfxcore.h>
#include <padinput.h>
#include <filepath.h>
#include <StringUtil.h>
#include <Hash.h>
#include <cell/DebugLog.h>
#include <cell/thread.h>

#include <thing.h>
#include <RenderTarget.h>
#include <Mesh.h>
#include <Poppet.h>
#include <PoppetChild.h>
#include <OverlayUI.h>
#include <Fart.h>
#include <View.h>
#include <GuidHashMap.h>
#include <GFXApi.h>
#include <Explode.h>
#include <LoadingScreen.h>
#include <InventoryView.h>
#include <InventoryItem.h>
#include <InventoryCollection.h>
#include <PoppetEnums.inl>

#include <PartPhysicsWorld.h>
#include <PartYellowHead.h>
#include <PartGeneratedMesh.h>

#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <ResourceGuidSubst.h>
#include <ResourceGFXFont.h>
#include <ResourceLocalProfile.h>
#include <ResourceSyncedProfile.h>
#include <ResourceTranslationTable.h>
#include <ResourceFileOfBytes.h>
#include <ResourceSystem.h>

#include <gooey/GooeyNodeManager.h>
#include <gooey/GooeyImage.h>
#include <network/NetworkManager.h>
#include <poppet/ScriptObjectPoppet.h>
#include <MMAudio.h>

#ifdef __SM64__
#include <sm64/init.h>
#include <sm64/avatar.h>
#endif

void DumpMeshToFile(CThing* thing)
{
    PGeneratedMesh* generated = thing->GetPGeneratedMesh();
    PPos* part_pos = thing->GetPPos();
    if (generated == NULL || part_pos == NULL) return;

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
}

void RenderShapeOutlines()
{
    EPlayerNumber leader = gNetworkManager.InputManager.GetLocalLeadersPlayerNumber();
    CThing* player = gGame->GetYellowheadFromPlayerNumber(leader);
    if (player == NULL) return;
    PYellowHead* yellowhead = player->GetPYellowHead();
    if (yellowhead == NULL) return;
    CPoppet* poppet = yellowhead->Poppet;
    if (poppet == NULL) return;

    cellGcmSetDepthTestEnable(gCellGcmCurrentContext, CELL_GCM_TRUE);
    cellGcmSetDepthFunc(gCellGcmCurrentContext, CELL_GCM_LEQUAL);
    for (int i = 0; i < gView.WorldList.size(); ++i)
    {
        PWorld* world = gView.WorldList[i];
        for (int j = 0; j < world->Things.size(); ++j)
        {
            CThing* thing = world->Things[j];
            poppet->RenderHoverObject(thing, 5.0f);
        }
    }
}

void OnReleaseLevel()
{
    #ifdef __SM64__
    ClearMarioAvatars();
    #endif
}

#include "AlearSync.h"

void OnResetPoppetModeStack(CPoppet* poppet)
{
    poppet->ClearHiddenList();
}

void CPoppet::ClearHiddenList()
{
    HiddenList.clear();
}

void OnUpdateLevel()
{
    MainThreadUpdate();

    // Tick Mario's in the world if enabled.
    #ifdef __SM64__
    UpdateMarioAvatars();
    #endif

    // Handle any reloads for databases and caches,
    // if anything was changed on disk or requested from
    // a local network.
    ReloadPendingDatabases();
    ProcessStartMenuNotifications();

    // Check if there are any mesh export requests from a player in the world
    PWorld* world = gGame->Level->WorldThing->GetPWorld();
    PYellowHead** it = world->ListPYellowHead.begin();
    for (PYellowHead** it = world->ListPYellowHead.begin(); it != world->ListPYellowHead.end(); ++it)
    {
        PYellowHead* yellowhead = *it;
        CPoppet* poppet = yellowhead->Poppet;
        if (poppet == NULL) continue;

        CInput* input = yellowhead->GetInput();
        if (input == NULL) continue;

        CThing* player = yellowhead->GetThing();
        // if (player != NULL && input->IsJustClicked(BUTTON_CONFIG_FORCE_BLAST, (const wchar_t*)NULL))
        // {
        //     ExplosionInfo info;
        //     GetExplosionInfo(player, info);
        //     info.Center = info.Center + v2(0.0f, -50.0f, 0.0f, 0.0f);
        //     info.IgnoreYellowHead = true;

        //     info.OuterRadius = 250.0f;
        //     info.InnerRadius = 250.0f;
        //     info.MaxVel = 100.0f;
        //     info.MaxForce = 1500.0f;
        //     info.MaxAngVel = 1.0f;

        //     ApplyRadialForce(info);
        // }

        CThing* hover = poppet->Edit.LastHoverThing;

        // if (poppet->GetMode() == MODE_CURSOR && poppet->GetSubMode() == SUBMODE_NONE)
        // {
        //     if (hover != NULL)
        //     {
        //         PShape* shape = hover->GetPShape();
        //         if (shape != NULL && input->IsJustClicked(BUTTON_CONFIG_POPPET_HIDE, L"HIDE"))
        //             poppet->HiddenList.push_back(hover);
        //     }

        //     if (poppet->HiddenList.size() != 0 && input->IsJustClicked(BUTTON_CONFIG_POPPET_SHOW, L"SHOW"))
        //         poppet->ClearHiddenList();
        // }

        if (hover != NULL && 
            poppet->GetMode() == MODE_CURSOR && 
            poppet->GetSubMode() == SUBMODE_NONE && 
            (gPadData->ButtonsDown & PAD_BUTTON_TRIANGLE) != 0)
        {
            // DumpMeshToFile(hover);
        }

    }

    UpdateItemRequest();

    // if ((gPadData->ButtonsDown & PAD_BUTTON_TRIANGLE) != 0)
    // {
    //     PYellowHead* yellowhead = world->ListPYellowHead[0];
    //     CThing* player_thing = yellowhead->GetThing();
    // }



}


void OnRunPipelinePostProcessing()
{
    gPoppetBloomHack.clear();
    
    #if __SM64__
    UpdateMarioDebugRender();
    #endif 
    
    if (gRenderOnlyPopit)
    {
        cellGcmSetClearColor(gCellGcmCurrentContext, 0x00000000);
        cellGcmSetClearSurface(gCellGcmCurrentContext, CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);
    }
    
    if (gShowOutlines) RenderShapeOutlines();
}

void OnPredictionOrRenderUpdate()
{
    UpdateDownloadInfo();
    
    if (!gView.DebugCameraActive)
        UpdateDebugCameraNotInUse();
    else
        UpdateDebugCamera();
    #if __SM64__
    UpdateMarioAvatarsRender();
    #endif
}

void OnWorldRenderUpdate()
{

}

void OnUpdateHttpTasks()
{
    gPinsTask.Update();
}

CVector<CP<RTranslationTable> > gTranslations;
bool CustomTryTranslate(u32 key, tchar_t const*& out)
{
    static tchar_t EMPTY_STRING[] = { 0x20 };

    if (gTranslations.size() == 0)
    {
        CP<RFileOfBytes> rlst = LoadResourceByKey<RFileOfBytes>(E_TRANSLATIONS_RLST, 0, STREAM_PRIORITY_DEFAULT);
        rlst->BlockUntilLoaded();

        CVector<MMString<char> > lines;
        LinesLoad(rlst->GetData(), lines, &StripAndIgnoreHash);

        for (int i = 0; i < lines.size(); ++i)
        {
            MMString<char>& line = lines[i];
            CFilePath fp(FPR_BLURAY, line.c_str());
            CP<RTranslationTable> subst = LoadResourceByFilename<RTranslationTable>(fp, 0, STREAM_PRIORITY_DEFAULT, false); 
            gTranslations.push_back(subst);
        }
        
        gTranslations.push_back(gPatchTrans);
        gTranslations.push_back(gTranslationTable);

        BlockUntilResourcesLoaded((CResource**)gTranslations.begin(), gTranslations.size());
    }

    for (int i = 0; i < gTranslations.size(); ++i)
    {
        RTranslationTable* table = gTranslations[i];
        if (table == NULL || !table->IsLoaded()) continue;
        if (table->GetText(key, out)) return true;
    }
    
    out = EMPTY_STRING;
    return false;
}

PlanDescriptorSet gUsedPlanDescriptors;
void GatherUsedPlanDescriptors()
{
    DebugLog("gathering used plan descriptors in level...\n");

    gUsedPlanDescriptors.clear();

    PWorld* world = gGame->GetWorld();
    if (world == NULL) return;

    for (CThing** it = world->Things.begin(); it != world->Things.end(); ++it)
    {
        CThing* thing = *it;
        if (thing == NULL) continue;

        if (thing->PlanGUID)
        {
            CResourceDescriptor<RPlan> desc(thing->PlanGUID);
            gUsedPlanDescriptors.insert(desc);
        }

        PGroup* group = thing->GetPGroup();
        if (group != NULL && group->PlanDescriptor.IsValid())
            gUsedPlanDescriptors.insert(group->PlanDescriptor);

        PRef* ref = thing->GetPRef();
        if (ref != NULL && ref->Plan.IsValid())
            gUsedPlanDescriptors.insert(ref->Plan);

        // PCostume* costume = thing->GetPCostume();
        // if (costume != NULL)
        // {
        //     if (costume->MatPlan.IsValid()) gUsedPlanDescriptors.insert(costume->MatPlan);
        //     for (int i = 0; i < COSTUMEPART_COUNT; ++i)
        //     {
        //         CCostumePiece& piece = costume->CostumePieceVec[i];
        //         if (piece.OriginalPlan.IsValid())
        //             gUsedPlanDescriptors.insert(piece.OriginalPlan);
        //     }
        // }

        PStickers* stickers = thing->GetPStickers();
        if (stickers != NULL)
        {
            for (CDecal* decal = stickers->Decals.begin(); decal != stickers->Decals.end(); ++decal)
            {
                if (decal->PlanGUID)
                {
                    CResourceDescriptor<RPlan> desc(decal->PlanGUID);
                    gUsedPlanDescriptors.insert(desc);
                }
            }

            for (int i = 0; i < COSTUMEPART_COUNT; ++i)
            {
                CVector<CDecal>& decals = stickers->CostumeDecals[i];
                for (CDecal* decal = decals.begin(); decal != decals.end(); ++decal)
                {
                    if (decal->PlanGUID)
                    {
                        CResourceDescriptor<RPlan> desc(decal->PlanGUID);
                        gUsedPlanDescriptors.insert(desc);
                    }
                }
            }
        }

        PDecorations* decorations = thing->GetPDecorations();
        if (decorations != NULL)
        {
            for (CDecoration* decor = decorations->Decorations.begin(); decor != decorations->Decorations.end(); ++decor)
            {
                if (decor->PlanGUID)
                {
                    CResourceDescriptor<RPlan> desc(decor->PlanGUID);
                    gUsedPlanDescriptors.insert(desc);
                }
            }
        }

        PGeneratedMesh* generated_mesh = thing->GetPGeneratedMesh();
        if (generated_mesh != NULL && generated_mesh->PlanGUID)
        {
            CResourceDescriptor<RPlan> desc(generated_mesh->PlanGUID);
            gUsedPlanDescriptors.insert(desc);
        }
    }

    // Refresh all poppets
    for (PYellowHead** it = world->ListPYellowHead.begin(); it != world->ListPYellowHead.end(); ++it)
    {
        PYellowHead* yellowhead = *it;
        CPoppet* poppet = yellowhead->Poppet;
        if (poppet == NULL) continue;
        const CP<RLocalProfile>& prf = poppet->GetLocalProfile();
        if (prf) prf->SetUsedItemViewDirty();
    }

    DebugLog("there are %d used descriptors in the level...\n", gUsedPlanDescriptors.size());
}

CVector<CP<RGuidSubst> > gSubsts;
bool CustomDoGUIDSubstitution(CGUID g, CGUID& o)
{
    RGuidSubst* substs[3] =
    {
        NGuidSubst::gRegionSubst,
        NGuidSubst::gButtonSubst,
        NGuidSubst::gLanguageSubst
    };

    o = g;

    for (int i = 0; i < gSubsts.size(); ++i)
    {
        RGuidSubst* subst = gSubsts[i];
        if (subst == NULL || !subst->IsLoaded()) continue;
        if (subst->Get(g, o)) return true;
    }

    for (int i = 0; i < ARRAY_LENGTH(substs); ++i)
    {
        RGuidSubst* subst = substs[i];
        if (subst == NULL || !subst->IsLoaded()) continue;
        if (subst->Get(g, o)) return true;
    }

    return false;
}

void OnLoadSubstTablesFinished()
{
    gSubsts.clear();

    CP<RFileOfBytes> rlst = LoadResourceByKey<RFileOfBytes>(E_GSUB_RLST, 0, STREAM_PRIORITY_DEFAULT);
    rlst->BlockUntilLoaded();

    CVector<MMString<char> > lines;
    LinesLoad(rlst->GetData(), lines, &StripAndIgnoreHash);

    for (int i = 0; i < lines.size(); ++i)
    {
        MMString<char>& line = lines[i];
        CFilePath fp(FPR_BLURAY, line.c_str());
        CP<RGuidSubst> subst = LoadResourceByFilename<RGuidSubst>(fp, 0, STREAM_PRIORITY_DEFAULT, false); 
        gSubsts.push_back(subst);
    }

    BlockUntilResourcesLoaded((CResource**)gSubsts.begin(), gSubsts.size());
}

bool IsItemSelected(RLocalProfile* profile, CInventoryItem* item)
{
    if (item->Details.SubType == E_SUBTYPE_ANIMATION_STYLE)
        return profile->SelectedAnimationStyle.GetGUID() == item->Plan.GetGUID();
    return profile->IsWearingCostumeUID(item->UID);
}

CInventoryItem* FindItemWithToolType(CBaseProfile* prf, EToolType type)
{
    for (CInventoryItem* it = prf->Inventory.begin(); it != prf->Inventory.end(); ++it)
    {
        if (it->Details.ToolType == type)
            return it;
    }

    return NULL;
}

void OnLocalProfileLoadFinished(RLocalProfile* prf)
{

}

FMOD_RESULT LoadAllEventProjects()
{
    for (int i = 0; i < CAudio::gFMODFileSize; ++i)
    {
        CAudio::FMODFile& file = CAudio::gFMODFiles[i];
        CFileDBRow* row = FileDB::FindByGUID(file.Key);
        if (row == NULL) continue;

        DebugLog("audiophile: %s\n", row->FilePathX);

        if (strstr(row->FilePathX, ".fev") == NULL) continue;
        CFilePath fp(FPR_GAMEDATA, row->FilePathX);

        FMOD::EventProject* project;
        FMOD_RESULT result = CAudio::EventSystem->load(fp.c_str(), NULL, &project);
        DebugLog("loaded fev at %s, errno=%08x\n", row->FilePathX, result);

        if (result != FMOD_OK) 
            return result;
    }

    return FMOD_OK;
}

void CustomPreRaycastPrepare(PWorld* world)
{
    gFilteredBucketForRaycast.Size = 0;
    if (world == NULL) return;

    for (int i = 0; i < gRenderBucket.Size; ++i)
    {
        CMeshInstance* instance = gRenderBucket.MeshInstance[i];
        CThing* thing = instance->MyThing;
        if (thing == NULL) continue;
        if (thing->Parts[PART_TYPE_JOINT] != NULL) continue;
        if (thing->GetPRenderMesh() == NULL) continue;

        gFilteredBucketForRaycast.MeshInstance[gFilteredBucketForRaycast.Size++] = instance;
    }
}

FileHandle gRecordingFileHandle = -1;
char fileData[1280 * 720 * 4];
bool gDoRecording;

MH_DefineFunc(Screenshot, 0x003e37b0, TOC1, char*, bool save_level_too, void* stream, char* prefix, u32 src, bool half_res, u32 override_index, u32 crop_width, u32 crop_height);
void OnSwapBuffers()
{
    if (!gDoRecording) return;
    
    if (gRecordingFileHandle == -1)
    {
        CFilePath fp(FPR_GAMEDATA, "output/rec.bin");
        FileOpen(fp, &gRecordingFileHandle, OPEN_WRITE);
    }

    NGfx::FlushGPU(true);

    char* pixel_data = NULL;
    int pixel_data_local;
    for (int i = 0; i < SMEM_POOL_LAST; ++i)
    {
        void* data = gSMemPools[i].Alloc(gResX * gResY * sizeof(u32), 16, 0);
        if (data != NULL)
        {
            pixel_data = (char*)data;
            pixel_data_local = gSMemPools[i].GetOffset(pixel_data);
            break;
        }
    }

    if (pixel_data == NULL) pixel_data = (char*)gPipelineRTs[PRT_TEMP_BACKBUFFER].Address;
    if (pixel_data == NULL) return;


    cellGcmSetTransferData(gCellGcmCurrentContext, CELL_GCM_TRANSFER_LOCAL_TO_MAIN, pixel_data_local, 1280 * 4, gBackBuffer[gCurBackBuffer].GetOffset(), 1280 * 4, 1280 * 4, 720);
    NGfx::FlushGPU(true);

    FileWrite(gRecordingFileHandle, pixel_data, 1280 * 720 * 4);
}

CP<CResource> TextureFromResource(CP<CResource>& res)
{
    if (!res) return NULL;
    EResourceType type = res->GetResourceType();
    if (type == RTYPE_TEXTURE || type == RTYPE_ANIMATED_TEXTURE) return res;
    return NULL;
}


bool CustomIsStickerPlaceable(void* edit, CRaycastResults const& results)
{
    if (results.SwitchConnector) return false;
    CThing* thing = results.HitThing;
    if (thing == NULL) return false;

    if (thing->GetPRenderMesh() == NULL && thing->GetPGeneratedMesh() == NULL) return false;
    return true;
}

void InitSharedHooks()
{
    // MH_Poke32(0x001c7b84, 0x4e800020);

    // MH_Poke32(0x00211730, 0x60000000);
    // MH_Poke32(0x00211700, 0x60000000);

    // MH_InitHook((void*)0x001ddb60, (void*)&CustomPreRaycastPrepare);
    // MH_InitHook((void*)0x0035d1a8, (void*)&CustomIsStickerPlaceable);

    MH_InitHook((void*)0x002eeb90, (void*)&CustomItemMatch);
    MH_InitHook((void*)0x000232bc, (void*)&CustomTryTranslate);
    MH_InitHook((void*)0x0009c52c, (void*)&CustomDoGUIDSubstitution);
    MH_PokeBranch(0x000b94e4, &_gsub_rlst_hook);
    MH_PokeBranch(0x003800b8, &_popit_isitemselected_hook);
    MH_PokeBranch(0x003e41b8, &_global_onswapbuffers_hook);

    MH_PokeBranch(0x00381354, &_popit_dopoppetsection_hook);
    MH_PokeBranch(0x0002018c, &_initextradata_cthing_hook);
    MH_PokeBranch(0x00022530, &_destroyextradata_cthing_hook);

    MH_PokeBranch(0x00234fcc, &_global_webternate_hook);

    // Increase the size of some inventory icons
    MH_PokeBranch(0x0037f90c, &_global_icon_size_hook);

    // Increase the icon generation size for icons
    MH_Poke32(0x0038916c, LI(5, 1));

    // Add hooks for allowing custom sort modes
    AttachCustomSortModes();

    // Add hooks for extra custom tool types
    AttachCustomToolTypes();
    AttachCustomPoppetMessages();
    MH_PokeBranch(0x003516e0, &_custom_pick_object_action_hook);
    MH_PokeBranch(0x001f0b3c, &_fady_thing_hook);
    MH_PokeBranch(0x00352100, &_fixup_custom_pick_object_select_hook);

    // Swap order of instructions in CGooeyNodeContainer constructor
    // so we can make sure our custom fields are always zero initialized
    MH_Poke32(0x002f3014, 0x917c00a4 /* stw %r11, 0xa4(%r28) */); // sneakily replace stb with stw
    MH_Poke32(0x002f3020, 0x993c00a5 /* stb %r9, 0xa5(%r28) */);

    // Allow overriding whether or not a frame will get clipped
    MH_PokeBranch(0x002f1d18, &_gooey_frame_clip_hook);

    // Draw custom buttons for sound objects
    MH_PokeBranch(0x00380180, &_custom_item_grid_hook);

    // Allow fevs in audiophiles rlst
    // MH_PokeBranch(0x001a4a98, &_custom_event_projects_hook);

    // woohoo weehee
    MH_PokeBranch(0x0034df5c, &_popit_close_hook);

    // Hooks for animated texture buttons
    // MH_Poke32(0x006ae1ac, LI(4, sizeof(CGooeyImage)));
    // MH_PokeBranch(0x003227d8, &_gooey_image_ctor_animated_hook);
    // MH_PokeBranch(0x00300268, &_sdf_button_animated_hook);
    // MH_PokeBranch(0x00322f94, &_gooey_image_update_hook);
    // MH_InitHook((void*)0x0043be2c, (void*)&TextureFromResource);

    // float v = -63600.0f;
    // MH_Poke32(0x008137c4, *((u32*)&v)); // SCREEN_MIN_Y
    // MH_Poke32(0x0091e430, *((u32*)&v)); // min thing pos y

    MH_PokeBranch(0xc2cf8, &_popit_alphabetical_hook);

    MH_PokeBranch(0x003428d4, &_popit_draw_cursor_hook);
    // MH_Poke32(0x0034c850, 0x60000000); // disable draw for bloom
    // MH_Poke32(0x0028ecb0, 0x42800008); // disable fat lines test

    // increase size of popit cursor sprite
    MH_Poke32(0x0092ac34, 0x420c0000);
    MH_Poke32(0x0092ac38, 0xc20c0000);

}

// Draw ( col, glitter, glitter_bloom, drawloop, drawtail, cam)