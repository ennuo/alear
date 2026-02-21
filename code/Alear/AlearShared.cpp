#include "AlearHooks.h"
#include "InventoryItemRequest.h"
#include "AlearShared.h"
#include "AlearStartMenu.h"
#include "AlearDebugCamera.h"
#include "AlearConfig.h"
#include "PinSystem.h"
#include "OutfitSystem.h"
#include "TweakShape.h"
#include "PoppetOutlineShapes.h"
#include "RenderJoint.h"
#include "AlearSerialization.h"

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
#include <ProfileCache.h>
#include <RenderTarget.h>
#include <Mesh.h>
#include <GameShell.h>
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
#include <ResourceGFXShader.h>
#include <ResourceSystem.h>

#include <gooey/GooeyNodeManager.h>
#include <gooey/GooeyImage.h>
#include <network/NetworkManager.h>
#include <network/NetworkPartiesData.h>
#include <poppet/ScriptObjectPoppet.h>
#include <MMAudio.h>

#ifdef __SM64__
#include <sm64/init.h>
#include <sm64/avatar.h>
#endif

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

        // CThing* player = yellowhead->GetThing();
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

        EPoppetMode mode = poppet->GetMode();
        EPoppetSubMode submode = poppet->GetSubMode();

        bool edit = gGame->EditMode;
        if ((poppet->GetMode() == MODE_CURSOR && 
                submode == SUBMODE_NONE ||
                submode == SUBMODE_EDIT_VERTS || 
                submode == SUBMODE_SWITCH_CONNECTOR || 
                submode == SUBMODE_DANGER || 
                submode == SUBMODE_FLOOD_FILL || 
                submode == SUBMODE_EYEDROPPER || 
                submode == SUBMODE_DOT_TO_DOT || 
                submode == SUBMODE_STICKER_CUTTER || 
                submode == SUBMODE_STICKER_SCRUBBER || 
                submode == SUBMODE_SLICE_N_DICE || 
                submode == SUBMODE_UNPHYSICS || 
                submode == SUBMODE_ADVANCED_GLUE || 
                submode == SUBMODE_EDIT_UVS || 
                submode == SUBMODE_MESH_CAPTURE) ||
            (poppet->GetMode() == MODE_CURSOR && submode == SUBMODE_PICK_DECORATIONS && edit))
        {
            if (hover != NULL)
            {
                PShape* shape = hover->GetPShape();
                if (shape != NULL && input->IsJustClicked(BUTTON_CONFIG_POPPET_HIDE, L"BP_HIDE"))
                {
                    poppet->HiddenList.push_back(hover);
                    CAudio::PlaySample(CAudio::gSFX, "poppet/layer_hide", hover, -10000.0f, -10000.0f);
                }
            }

            if (poppet->HiddenList.size() != 0 && input->IsJustClicked(BUTTON_CONFIG_POPPET_SHOW, L"BP_SHOW"))
            {
                poppet->ClearHiddenList();
                v2 pos3d = *(v2*)(((char*)poppet) + 0x180);
                CAudio::PlaySample(CAudio::gSFX, "poppet/layer_unhide", -10000.0f, &pos3d, -10000.0f);
            }
        }
        
        if (gCanHidePopit && input->IsJustClicked(BUTTON_CONFIG_POPPET_TOGGLE_TETHER_UI, L"BP_HIDE_TETHER"))
            poppet->ShowTether = !poppet->ShowTether;

        if (gCanHidePopit && input->IsJustClicked(BUTTON_CONFIG_POPPET_TOGGLE_INVENTORY_UI, L"BP_HIDE_POPPET_UI"))
            poppet->HidePoppetGooey = !poppet->HidePoppetGooey;

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


extern void RenderSwitchDebug();
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
    RenderSwitchDebug();
}

void OnPredictionOrRenderUpdate()
{
    UpdateDownloadInfo();
    UpdatePinOverlay();
    
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
    // gPinsTask.Update();
}

CVector<CP<RTranslationTable> > gTranslations;
bool CustomTryTranslate(u32 key, tchar_t const*& out)
{
    if (key == MakeLamsKeyID("BP_", "HIDE_TETHER"))
    {
        out = (tchar_t*)L"Hide Tether";
        return true;
    }

    if (key == MakeLamsKeyID("BP_", "HIDE_POPPET_UI"))
    {
        out = (tchar_t*)L"Hide Poppet UI";
        return true;
    }

    if (key == E_LAMS_TWEAKABLE_MATERIAL)
    {
        out = (tchar_t*)L"Material";
        return true;
    }

    if (key == E_LAMS_TWEAKABLE_MESH)
    {
        out = (tchar_t*)L"Mesh";
        return true;
    }
    
    if (key == E_LAMS_TWEAKABLE_DECAL)
    {
        out = (tchar_t*)L"Decal";
        return true;
    }

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

void AddToolToInventory(RLocalProfile* prf, const char* name, EToolType type, u32 icon_key)
{
    const u32 E_FUNCTIONS_KEY = 1648289136;
    const u32 E_KEY_IMPORT_TOOL = 65717;

    CP<RTexture> icon = LoadResourceByKey<RTexture>(icon_key, 0, STREAM_PRIORITY_DEFAULT);

    CInventoryItem item;
    item.UID = prf->NextInventoryItemUID++;
    item.Plan = CResourceDescriptor<RPlan>(E_KEY_IMPORT_TOOL);

    CInventoryItemDetails& details = item.Details;
    details.Type = E_TYPE_TOOL;
    details.ToolType = type;
    strcpy(details.Creator.handle.data, "MM_Studio");
    details.NameTranslationTag = MakeLamsKeyID(name, "_NAME");
    details.DescTranslationTag = MakeLamsKeyID(name, "_DESC");
    details.SetIcon(icon);
    details.LocationIndex = prf->AddString(E_FUNCTIONS_KEY);
    details.CategoryIndex = prf->AddString(E_FUNCTIONS_KEY);
    details.NumUses = 1;

    CRawVector<CInventoryItem>& items = *((CRawVector<CInventoryItem>*)&prf->Inventory);
    items.push_back(item);
}


void OnLocalProfileLoadFinished(RLocalProfile* prf)
{



    AddToolToInventory(prf, "POPPET_TOOL_PLAN_TAKE", TOOL_SHAPE_TAKE_PLAN, 42929u);
    AddToolToInventory(prf, "POPPET_TOOL_VERTEX_EDIT", TOOL_SHAPE_VERTEX_EDIT, 42407u);
    AddToolToInventory(prf, "POPPET_TOOL_SLICE_N_DICE", TOOL_SLICE_N_DICE, 3646450585u);
    AddToolToInventory(prf, "POPPET_TOOL_DOT_TO_DOT", TOOL_DOT_TO_DOT, 2762794371u);
    AddToolToInventory(prf, "POPPET_TOOL_EYEDROPPER", TOOL_EYEDROPPER, 2693374007u);
    AddToolToInventory(prf, "POPPET_TOOL_FLOOD_FILL", TOOL_SHAPE_FLOOD_FILL, 42406u);
    AddToolToInventory(prf, "POPPET_TOOL_UV_EDIT", TOOL_UV_EDIT, 3373407558u);
    AddToolToInventory(prf, "POPPET_TOOL_GLUE", TOOL_GLUE, 2739668065u);
    AddToolToInventory(prf, "POPPET_TOOL_UNPHYSICS", TOOL_UNPHYSICS, 3430870302u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_UNLETHAL", TOOL_SHAPE_UNLETHAL, 42936u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_FIRE", TOOL_SHAPE_BURNINATE, 42933u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_ELECTRIC", TOOL_SHAPE_ELECTRIFY, 42932u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_ICE", TOOL_SHAPE_ICE, 42934u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_POISONGAS", TOOL_SHAPE_GAS, 42935u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_PLASMA", TOOL_SHAPE_PLASMA, 3461868587u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_SPIKE", TOOL_SHAPE_SPIKE, 3136424487u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_CRUSH", TOOL_SHAPE_CRUSH, 3664313855u);
    AddToolToInventory(prf, "POPPET_TOOL_DANGER_DROWN", TOOL_SHAPE_DROWNED, 3664313855u);
    AddToolToInventory(prf, "POPPET_TOOL_STICKER_PICK", TOOL_STICKER_PICK, 42408u);
    AddToolToInventory(prf, "POPPET_IMPORT_PHOTO_FROM_XMB", TOOL_DECORATION_IMPORT_PHOTO, 65645u);
}


void OnBaseProfileLoadFinished(CBaseProfile* prf)
{
    return;

    // Delete all tools from the inventory, we're going to re-add them in sorted order.
    // CRawVector<CInventoryItem>& items = *((CRawVector<CInventoryItem>*)&prf->Inventory);
    // for (CInventoryItem* item = items.begin(); item != items.end(); ++item)
    // {
    //     if (item->Details.ToolType != TOOL_NOT_A_TOOL)
    //         item = items.erase(item) - 1;
    // }

    // if (prf->GetResourceType() == RTYPE_LOCAL_PROFILE)
    //     OnLocalProfileLoadFinished((RLocalProfile*)prf);
}

AUDIO_GROUP gStingerGroup;

FMOD_RESULT LoadAllEventProjects()
{
    for (int i = 0; i < CAudio::gFMODFileSize; ++i)
    {
        CAudio::FMODFile& file = CAudio::gFMODFiles[i];
        CFileDBRow* row = FileDB::FindByGUID(file.Key);
        if (row == NULL) continue;

        if (strstr(row->FilePathX, ".fev") == NULL) continue;
        CFilePath fp(FPR_GAMEDATA, row->FilePathX);

        FMOD::EventProject* project;
        FMOD_RESULT result = CAudio::EventSystem->load(fp.c_str(), NULL, &project);
        DebugLog("loaded fev at %s, errno=%08x\n", row->FilePathX, result);

        

        if (result != FMOD_OK) 
            return result;
    }

    FMOD_RESULT result = CAudio::EventSystem->getGroup("stings/music/stings", FMOD_DEFAULT, &gStingerGroup.t);
    if (result != FMOD_OK) return result;
    gStingerGroup.Frame = gGraphicsFrameNum;

    return FMOD_OK;
}

bool IsGamePaused()
{
    if (GetGamePartyData(E_CHECK_THREAD).NumPS3s() < 2)
    {
        if (gPauseGameSim) return true;

        CP<RLocalProfile>& prf = ProfileCache::GetOrCreateMainUserProfile();
        if ((prf && gGameShell->OverlayUI->IsStartMenuOpen()) || 
            gNetworkManager.GameDataManager.UserSelectorForPad.IsSelectionScreenCreated()) 
            return true;

        return SomePadHasInputIntercepted();
    }

    return false;
}


namespace NPoppetUtils {
    MH_DefineFunc(UseSphereRaycast, 0x0038edb8, TOC1, bool, CThing const* thing);
}

extern bool IsThingFady(CThing* thing);
void CustomPreRaycastPrepare(PWorld* world)
{
    gFilteredBucketForRaycast.Size = 0;
    if (world == NULL) return;

    CThing* backdrop = world->Backdrop;
    CThing* backdrop_new = world->BackdropNew;

    for (int i = 0; i < gRenderBucket.Size; ++i)
    {
        CMeshInstance* instance = gRenderBucket.MeshInstance[i];
        CThing* thing = instance->MyThing;

        if (thing == NULL) continue;
        if (NPoppetUtils::UseSphereRaycast(thing)) continue;
        if (backdrop != NULL && thing->Parent == backdrop) continue;
        if (backdrop_new != NULL && thing->Parent == backdrop_new) continue;
        if (thing->World != world) continue;
        if (IsThingFady(thing)) continue;

        gFilteredBucketForRaycast.MeshInstance[gFilteredBucketForRaycast.Size++] = instance;
    }
}
StaticCP<RPixelShader> gCopyGlowShader;
StaticCP<RPixelShader> gRenderPoppetShader;
StaticCP<RVertexShader> gFullscreenShader;
ByteArray gVideoHeader;

void LoadRecordingShaders()
{
    CFilePath fp(FPR_GAMEDATA, "gamedata/alear/data/avi.raw");
    CHash hash;
    FileLoad(fp, gVideoHeader, hash);

    *((CP<RPixelShader>*)&gCopyGlowShader) = LoadResourceByKey<RPixelShader>(3306909899u, 0, STREAM_PRIORITY_DEFAULT);
    *((CP<RPixelShader>*)&gRenderPoppetShader) = LoadResourceByKey<RPixelShader>(3364422394u, 0, STREAM_PRIORITY_DEFAULT);
     *((CP<RVertexShader>*)&gFullscreenShader) = LoadResourceByKey<RVertexShader>(19194, 0, STREAM_PRIORITY_DEFAULT);
}

void CVideoRecording::StartRecording(char* path)
{
    if (IsRecording()) StopRecording();
    NumFrames = 0;
    Filepath.Assign(FPR_GAMEDATA, path);
    if (FileOpen(Filepath, &Handle, OPEN_RDWR))
    {
        FileResize(Handle, gVideoHeader.size());
        FileSeek(Handle, gVideoHeader.size(), CELL_FS_SEEK_SET);
    }
}

void CVideoRecording::StopRecording()
{
    if (!IsRecording()) return;

    u32 data_len = (gResX * gResY * sizeof(s32)) * NumFrames + (0xc * NumFrames);
    u32 file_len = gVideoHeader.size() + data_len;
    u32 riff_size = file_len - 0x8;
    u32 mov_size = data_len + 0xc;
    
    *(u32*)(gVideoHeader.begin() + 0x004) = (riff_size >> 24) | ((riff_size << 8) & 0x00FF0000) | ((riff_size >> 8) & 0x0000FF00) | (riff_size << 24);
    *(u32*)(gVideoHeader.begin() + 0x030) = (NumFrames >> 24) | ((NumFrames << 8) & 0x00FF0000) | ((NumFrames >> 8) & 0x0000FF00) | (NumFrames << 24);
    *(u32*)(gVideoHeader.begin() + 0x08c) = (NumFrames >> 24) | ((NumFrames << 8) & 0x00FF0000) | ((NumFrames >> 8) & 0x0000FF00) | (NumFrames << 24);
    *(u32*)(gVideoHeader.begin() + 0x5c8) = (mov_size >> 24) | ((mov_size << 8) & 0x00FF0000) | ((mov_size >> 8) & 0x0000FF00) | (mov_size << 24);
    
    FileSeek(Handle, 0, CELL_FS_SEEK_SET);
    FileWrite(Handle, (void*)gVideoHeader.begin(), gVideoHeader.size());
    FileClose(&Handle);
}

bool CVideoRecording::IsRecording()
{
    return Handle != -1;
}

void CVideoRecording::ToggleRecording(char* path)
{
    if (IsRecording())
    {
        StopRecording();
        return;
    }

    StartRecording(path);
}

void CVideoRecording::AppendVideoFrame()
{
    CRenderTarget& target = gPipelineRTs[PRT_TEMP_BACKBUFFER];
    NGfx::FlushGPU(true);

    char chunk[] = { '0', '0', 'd', 'b', '\0', '\0', '\0', '\0' };
    u32 frame_size = (gResX * gResY * sizeof(s32));
    *(u32*)(chunk + 4) = (frame_size >> 24) | ((frame_size << 8) & 0x00FF0000) | ((frame_size >> 8) & 0x0000FF00) | (frame_size << 24);
    FileWrite(Handle, (void*)chunk, 0x8);

    for (int i = 0; i < gResX * gResY; ++i)
    {
        u32& col = *((u32*)target.Address + i);
        col = (col >> 24) | ((col << 8) & 0x00FF0000) | ((col >> 8) & 0x0000FF00) | (col << 24);
    }

    FileWrite(Handle, target.Address, gResX * gResY * sizeof(u32));
    NumFrames++;
}

CVideoRecording gBloomRecording;
CVideoRecording gPoppetRecording;


MH_DefineFunc(Screenshot, 0x003e37b0, TOC1, char*, bool save_level_too, void* stream, char* prefix, u32 src, bool half_res, u32 override_index, u32 crop_width, u32 crop_height);
MH_DefineFunc(GenericPNGWrite, 0x001ce450, TOC0, bool, void* fp, ByteArray& buffer, void* stream, void* image_data, u32 width, u32 height, u32 pitch);
void OnSwapBuffers()
{
    CRenderTarget& target = gPipelineRTs[PRT_TEMP_BACKBUFFER];
    CRenderTarget& cbuf = gPipelineRTs[PRT_C_BUFFER_MSAA];

    NGfx::gTgUseNormals = false;

    if (gBloomRecording.IsRecording())
    {
        SetRenderTarget(gPipelineBindings[PRT_TEMP_BACKBUFFER]);

        cellGcmSetClearColor(gCellGcmCurrentContext, 0x00000000);
        cellGcmSetClearSurface(gCellGcmCurrentContext, CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);

        gPipelineRTs[PRT_BLOOM_BUFFER_64P].BindAsTexture(0, &CRenderTarget::gLinear_Clamp);
        NGfx::BindShader(gCopyGlowShader, true);
        NGfx::BindShader(gFullscreenShader, true);
        RenderFullscreenQuad(0xFFFFFFFF, false, false, 0.5f);

        gBloomRecording.AppendVideoFrame();
    }

    if (gPoppetRecording.IsRecording())
    {
        SetRenderTarget(gPipelineBindings[PRT_C_BUFFER_MSAA]);
        cellGcmSetClearColor(gCellGcmCurrentContext, 0x00000000);
        cellGcmSetClearSurface(gCellGcmCurrentContext, CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);

        PWorld* world = gGame->GetWorld();
        if (world != NULL)
        {
            for (PYellowHead** it = world->ListPYellowHead.begin(); it != world->ListPYellowHead.end(); ++it)
            {
                PYellowHead* yellowhead = *it;
                if (yellowhead == NULL) continue;

                CPoppet* poppet = yellowhead->Poppet;
                if (poppet == NULL) continue;

                poppet->RenderUI();
            }
        }

        SetRenderTarget(gPipelineBindings[PRT_TEMP_BACKBUFFER]);

        cellGcmSetClearColor(gCellGcmCurrentContext, 0x00000000);
        cellGcmSetClearSurface(gCellGcmCurrentContext, CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);

        gPipelineRTs[PRT_C_BUFFER_MSAA].BindAsTexture(0, &CRenderTarget::gLinear_Clamp);
        NGfx::BindShader(gFullscreenShader, true);
        NGfx::BindShader(gRenderPoppetShader, true);
        RenderFullscreenQuad(0xFFFFFFFF, false, false, 0.5f);
        gPoppetRecording.AppendVideoFrame();
    }

    SetNiceState();
    SetRenderTargetBackBuffer(gCurBackBuffer);
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

extern bool CanTweakThing(CPoppet* poppet, CThing* thing);


extern void AttachPoppetInterfaceExtensionHooks();

enum AudioSfxType {
    AUDIO_SFX_FIRE,
    AUDIO_SFX_ELECTRICITY,
    AUDIO_SFX_GAS,
    AUDIO_SFX_ICE,
    AUDIO_SFX_PLASMA,
    AUDIO_SFX_NONE,
    AUDIO_SFX_NUM_TYPES
};

AudioSfxType MapELethalTypeToAudioType(ELethalType type)
{
    switch (type)
    {
        case LETHAL_FIRE: 
            return AUDIO_SFX_FIRE;
        case LETHAL_BULLET: 
            return AUDIO_SFX_PLASMA;
        case LETHAL_POISON_GAS:
        case LETHAL_POISON_GAS2:
        case LETHAL_POISON_GAS3:
        case LETHAL_POISON_GAS4:
        case LETHAL_POISON_GAS5:
        case LETHAL_POISON_GAS6:
            return AUDIO_SFX_GAS;
        case LETHAL_ELECTRIC:
            return AUDIO_SFX_ELECTRICITY;
        case LETHAL_ICE:
            return AUDIO_SFX_ICE;
        default:
            return AUDIO_SFX_NONE;
    }
}

bool gHackIsDrawingIceyThings;
StaticCP<RPixelShader> gFirePixelShader;
StaticCP<RPixelShader> gIcePixelShader;

ELethalType GetFrizzleFryLethalType()
{
    return gHackIsDrawingIceyThings ? LETHAL_ICE : LETHAL_FIRE;
}

bool IsFrizzleFryLethal(ELethalType type)
{
    return type == GetFrizzleFryLethalType();
}

StaticCP<RPixelShader>& GetIceFirePixelShader()
{
    gHackIsDrawingIceyThings = !gHackIsDrawingIceyThings;
    return gHackIsDrawingIceyThings ? gIcePixelShader : gFirePixelShader;
}

void ResetLethalTimers(PShape* shape)
{
    ELethalType type = shape->LethalType;
    
    if (type == LETHAL_NOT) return;

    if (type == LETHAL_ELECTRIC || LETHAL_BULLET) 
        shape->ElectricFrame = 1;
    else if (type < 4) {
        if (type == LETHAL_FIRE || type == LETHAL_ICE)
            shape->FireFrame = 1;
    }
    else if (type - LETHAL_POISON_GAS < 6)
        shape->GasFrame = 1;
}

void LoadPostProcessingShaders()
{
    *((CP<RPixelShader>*)&gFirePixelShader) = LoadResourceByKey<RPixelShader>(31194, 0, STREAM_PRIORITY_DEFAULT);
    *((CP<RPixelShader>*)&gIcePixelShader) = LoadResourceByKey<RPixelShader>(31195, 0, STREAM_PRIORITY_DEFAULT);
}


v2 Hack_GetWalkInput(PCreature* creature)
{
    if (creature->State == STATE_FROZEN)
    {
        //if (creature->Fork->AmountBodySubmerged > 0.001f || creature->Fork->AmountHeadSubmerged > creature->Config->AmountSubmergedToNotBreath)
            return v2(0.0f);
    }

    return creature->Fork->WalkInput;
}

bool Hack_AvoidsObstacle(PCreature* creature, float f1, float f2, float f3, float f4, float f5)
{
    if (creature->State == STATE_FROZEN) return false;
    return (f1 <= 0.0f && f2 <= f4 - 170.0f) || (f5 + 170.0f <= f3);
}

// Ice needs (assuming these need hooks) fixes for the following:
// Remove layer controls when frozen
// Remove automatic layer shifting onto platforms
// Allow player to shift layers when on ice
// Disable player from putting hand on wall
// Disable footstep sounds when frozen? (not sure if this needs hook)
// Disable bubble emitter from mouth when frozen
// Allow player to open popit when frozen in create mode
// Allow player to unhover when frozen while just exiting water
// Call for fire head stun (related to standing on ice in normal state)

// Additional fix:
// Remove player avoiding ice (layer shifts when falling projectile is in the way)

// Questions for Aidan:
// Where do I get the player's velocity? I need this to check if the player is below a certain speed while frozen


void REIMPL_CSackBoyAnim_DoDeathAnims(CSackBoyAnim* self)
{
    self->DoDeathAnims();
}

void AttachIceHooks()
{
    MH_Poke32(0x003c3e60, NOP); // Don't unlethalize early ice hazards
    MH_PokeHook(0x001ab81c, MapELethalTypeToAudioType); // Play ice sounds when hazard is applied
    MH_PokeBranch(0x0041717c, &_get_frizzlefry_shader_hook);
    MH_PokeBranch(0x001dd96c, &_draw_icey_things_hook);
    MH_PokeBranch(0x004172a0, &_is_frizzlefry_lethal_hook);
    MH_PokeBranch(0x00417450, &_track_frizzlefry_lethal_hook);
    MH_PokeBranch(0x00348098, &_set_danger_type_ice_timer_hook);
    MH_PokeBranch(0x001c5180, &_draw_mesh_boundary_ice_hook);

    MH_Poke32(0x000ea4fc, LI(4, sizeof(CSackBoyAnim)));
    MH_PokeBranch(0x000fecf0, &_anim_update_ice_hook);
    MH_PokeBranch(0x000f656c, &_anim_choose_idle_ice_hook);
    MH_PokeBranch(0x000f2394, &_death_anim_ice_hook);

    // dumb hack to zero initialize fields in creature fork
    // expand thingptr initialization 
    MH_Poke32(0x0003e934, LI(8, 0xc6));

    // float
    MH_PokeBranch(0x000a5a70, &_creature_frozen_buoyancy_hook);

    MH_PokeHook(0x0002480c, Hack_GetWalkInput);

    MH_PokeBranch(0x000feba4, &_sackboy_anim_late_update_hook);

    // Pass WasFrozen into the OnDeath so we can trigger steam or smoke particles
    MH_Poke32(0x00072000, 0x80a30cfc /* lwz %r5, 0xcfc(%r3) */);
    // MH_Poke32(0x00072000, LI(5, 1));

    // Don't apply leg IK fixes to Sackboy when doing thaw animation.
    MH_PokeBranch(0x000fdeb8, &_sackboy_anim_ice_ik_hook);

    // Fixup ground normal update function setting ground distance to FLT_INF
    // when we're standing on an ice lethalized shape.
    MH_PokeBranch(0x000408fc, &_sackboy_ground_distance_ice_hook);

    // Don't avoid obstacles while frozen in a block
    MH_PokeHook(0x00024754, Hack_AvoidsObstacle);

    MH_PokeHook(0x000f2334, REIMPL_CSackBoyAnim_DoDeathAnims);
    MH_PokeMemberHook(0x000f1d40, CSackBoyAnim::Burnilate);
    MH_PokeMemberHook(0x000f2198, CSackBoyAnim::Gasify);
    MH_PokeMemberHook(0x000f172c, CSackBoyAnim::DeathSmoke);
    MH_PokeMemberHook(0x000f16ac, CSackBoyAnim::Steam);
}

void AttachRenderJointHooks()
{
    MH_PokeHook(0x0003e3a0, GetRenderJointMesh);
}

AUDIO_HANDLE gBackdropAudioHandle;
void OnBackdropChange(PWorld* world)
{
    world->InitBackdropCuller();
    if (!gPlayBackgroundStings || CAudio::GetCurrMusicGuid()) return;
    
    const char* stinger = NULL;
    switch (world->Backdrop->GetPRef()->Plan.GetGUID().guid)
    {
        default: 
            stinger = NULL; break;
        case 31968:
            stinger = "sting_empty_world"; break;
        case 31964:
            stinger = "sting_english_garden"; break;
        case 32026:
            stinger = "sting_english_seaside"; break;
        case 33684:
            stinger = "sting_african_savannah"; break;
        case 31967:
            stinger = "sting_mexico_graveyard"; break;
        case 31966:
            stinger = "sting_mexico_desert"; break;
        case 33371:
            stinger = "sting_american_midwest"; break;
        case 34409:
            stinger = "sting_american_newyork"; break;
        case 31970:
            stinger = "sting_japanese_window"; break;
        case 31965:
            stinger = "sting_japanese_zen"; break;
        case 32025:
            stinger = "sting_indian_jungle"; break;
        case 34408:
            stinger = "sting_russian_siberia"; break;
        case 32845:
            stinger = "sting_russian_theatre"; break;
        case 52330:
            stinger = "sting_moon"; break;
        case 66082:
            stinger = "sting_001_mgs"; break;
        case 69625:
            stinger = "sting_001_mpcave"; break;
        case 74443:
            stinger = "sting_001_history"; break;
        case 79106:
            stinger = "sting_001_incredibles"; break;
        case 79094:
            stinger = "sting_002_pirates"; break;
        case 92664:
            stinger = "sting_002_marvel"; break;
        case 80117:
            stinger = "sting_002_prehistoric"; break;
        case 92663:
            stinger = "sting_002_ff7"; break;
        case 96503:
            stinger = "LBP2/sting_davinci"; break;
        case 89577:
            stinger = "LBP2/sting_victoria"; break;
        case 89648:
            stinger = "LBP2/sting_clive"; break;
        case 83041:
            stinger = "LBP2/sting_avalon"; break;
        case 89967:
            stinger = "LBP2/sting_eve"; break;
        case 94369:
            stinger = "LBP2/sting_cosmos"; break;
    }

    CAudio::StopSample(gBackdropAudioHandle);
    if (stinger == NULL) return;
    gBackdropAudioHandle = CAudio::PlaySample(gStingerGroup, stinger, world->GetThing(), -10000.0f, -10000.0f);
}

void InitSharedHooks()
{
    MH_PokeBranch(0x001f7f68, &_draw_call_hook); 
    
    MH_PokeBranch(0x0014e0ec, &_debug_camera_input_hook);

    MH_PokeCall(0x0007c710, OnBackdropChange);
    AttachPoppetInterfaceExtensionHooks();
    AttachIceHooks();
    AttachRenderJointHooks();
    AttachSerializationHooks();

    MH_PokeHook(0x0035ac5c, LoadOutlinePolygons);
    MH_PokeBranch(0x003590b4, &_get_outline_guid_hook);
    

    // Disable scale list
    MH_PokeBranch(0x0036add4, &_can_scale_guid_list_hook);
    //MH_PokeBranch(0x000372a8, &_water_boost_disable_swimming_fins_hook);

    // MH_Poke32(0x001c71a0, BLR);
    // MH_Poke32(0x001c5180, 0x2f890003);
    // MH_Poke32(0x001c535c, 0x39600004);
    
    // MH_Poke32(0x001c7b84, 0x4e800020);

    // MH_Poke32(0x00211730, 0x60000000);
    // MH_Poke32(0x00211700, 0x60000000);

    MH_InitHook((void*)0x001ddb60, (void*)&CustomPreRaycastPrepare);
    MH_InitHook((void*)0x0035d1a8, (void*)&CustomIsStickerPlaceable);

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
    MH_PokeBranch(0x003545f0, &_custom_render_marquee_hook);
    MH_PokeBranch(0x0034b5dc, &_custom_update_marquee_hook);
    MH_PokeBranch(0x00351948, &_custom_marquee_selection_hook);
    MH_PokeBranch(0x00351950, &_custom_marquee_action_hook);
    MH_PokeBranch(0x0034fe68, &_disable_gas_tweak_hook);
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
    MH_PokeBranch(0x001a4a98, &_custom_event_projects_hook);

    // woohoo weehee
    MH_PokeBranch(0x0034df5c, &_popit_close_hook);

    // Hooks for animated texture buttons
    MH_Poke32(0x006ae1ac, LI(4, sizeof(CGooeyImage)));
    MH_PokeBranch(0x003227d8, &_gooey_image_ctor_animated_hook);
    MH_PokeBranch(0x00300268, &_sdf_button_animated_hook);
    MH_PokeBranch(0x00322f94, &_gooey_image_update_hook);
    MH_InitHook((void*)0x0043be2c, (void*)&TextureFromResource);

    // float v = -63600.0f;
    // MH_Poke32(0x008137c4, *((u32*)&v)); // SCREEN_MIN_Y
    // MH_Poke32(0x0091e430, *((u32*)&v)); // min thing pos y

    // MH_PokeBranch(0xc2cf8, &_popit_alphabetical_hook);

    MH_PokeBranch(0x003428d4, &_popit_draw_cursor_hook);
    // MH_Poke32(0x0034c850, 0x60000000); // disable draw for bloom
    // MH_Poke32(0x0028ecb0, 0x42800008); // disable fat lines test

    // increase size of popit cursor sprite
    MH_Poke32(0x0092ac34, 0x420c0000);
    MH_Poke32(0x0092ac38, 0xc20c0000);

    MH_PokeBranch(0x000b2690, &_base_profile_load_hook);
    
    // allow disabling rendering popit tether
    // DrawForBloom
    MH_Poke32(0x0034c870, 0x891b1b31 /* lbz %r8, 0x1b31(%r27) */);
    MH_Poke32(0x0034c604, 0x891b1b31 /* lbz %r8, 0x1b31(%r27) */);
    // RenderUI
    MH_Poke32(0x00345d84, 0x891d1b31 /* lbz %r8, 0x1b31(%r29) */);
    MH_Poke32(0x00345dc0, 0x891d1b31 /* lbz %r8, 0x1b31(%r29) */);

    MH_PokeBranch(0x00345a9c, &_popit_render_ui_debug_hook);

    MH_PokeBranch(0x0034fcd8, &_popit_attempt_tweak_hook);
    MH_InitHook((void*)0x003400c4, (void*)&CanTweakThing);

    MH_PokeHook(0x0009a634, IsGamePaused);
}

// Draw ( col, glitter, glitter_bloom, drawloop, drawtail, cam)