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
        if (player != NULL && input->IsJustClicked(BUTTON_CONFIG_FORCE_BLAST, (const wchar_t*)NULL))
        {
            ExplosionInfo info;
            GetExplosionInfo(player, info);
            info.Center = info.Center + v2(0.0f, -50.0f, 0.0f, 0.0f);
            info.IgnoreYellowHead = true;

            info.OuterRadius = 250.0f;
            info.InnerRadius = 250.0f;
            info.MaxVel = 100.0f;
            info.MaxForce = 1500.0f;
            info.MaxAngVel = 1.0f;

            ApplyRadialForce(info);
        }



        CThing* hover = poppet->Edit.LastHoverThing;

        if (poppet->GetMode() == MODE_CURSOR && poppet->GetSubMode() == SUBMODE_NONE)
        {

            if (hover != NULL)
            {
                PShape* shape = hover->GetPShape();
                if (shape != NULL && input->IsJustClicked(BUTTON_CONFIG_POPPET_HIDE, L"HIDE"))
                    poppet->HiddenList.push_back(hover);
            }

            if (poppet->HiddenList.size() != 0 && input->IsJustClicked(BUTTON_CONFIG_POPPET_SHOW, L"SHOW"))
                poppet->ClearHiddenList();
        }

        if (hover != NULL && 
            poppet->GetMode() == MODE_CURSOR && 
            poppet->GetSubMode() == SUBMODE_NONE && 
            (gPadData->ButtonsDown & PAD_BUTTON_TRIANGLE) != 0)
        {
            // DumpMeshToFile(hover);
        }

    }

    UpdateItemRequest();

    if ((gPadData->ButtonsDown & PAD_BUTTON_TRIANGLE) != 0)
    {
        PYellowHead* yellowhead = world->ListPYellowHead[0];
        CThing* player_thing = yellowhead->GetThing();
    }



}


void OnRunPipelinePostProcessing()
{
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

const u32 gUserObjectMask = 0x40100480;
const u32 gPodMask = E_TYPE_USER_POD | E_TYPE_POD_TOOL;

bool IsPlayerCustomizationTool(u32 tool_type)
{
    return 
        tool_type == TOOL_COSTUME_RESET ||
        tool_type == TOOL_COSTUME_RANDOM ||
        tool_type == TOOL_COSTUME_SAVE ||
        tool_type == TOOL_COSTUME_WASH;
}

bool IsToolMatch(CInventoryView* view, u32 tool_type)
{
    bool edit = gGame->EditMode;
    u32 type = view->Descriptor.Type;
    u32 subtype = view->Descriptor.SubType;

    switch (tool_type)
    {
        case TOOL_COSTUME_RESET:
        case TOOL_COSTUME_RANDOM:
        case TOOL_COSTUME_SAVE:
        case TOOL_COSTUME_WASH:
        {
            return (type & (E_TYPE_COSTUME | E_TYPE_COSTUME_MATERIAL | E_TYPE_USER_COSTUME)) != 0;
        }

        case TOOL_STICKER_PICK:
        {
            return (type & (E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_EYETOY)) != 0;
        }

        case TOOL_DECORATION_TAKE_PHOTO:
        {
            return (type & (E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_EYETOY)) != 0;
        }

        case TOOL_DELETE_COMMUNITY_STICKERS:
        {
            return (type & E_TYPE_USER_STICKER) != 0 && (subtype & E_SUBTYPE_MADE_BY_OTHERS) != 0;
        }

        case TOOL_POD_RESET:
        case TOOL_POD_SAVE:
        {
            return (type & E_TYPE_USER_POD) != 0;
        }

        case TOOL_SHAPE_TAKE_PLAN:
        {
            return (type & (E_TYPE_PRIMITIVE_MATERIAL | E_TYPE_READYMADE | E_TYPE_USER_OBJECT | E_TYPE_TOOL)) != 0;
        }

        case TOOL_DELETE_COMMUNITY_OBJECTS:
        {
            return (type & E_TYPE_USER_OBJECT) != 0 && (subtype & E_SUBTYPE_MADE_BY_OTHERS) != 0;
        }

        case TOOL_DECORATION_IMPORT_PHOTO:
        {
            return (type & E_TYPE_USER_STICKER) != 0;
        }

        case TOOL_SHAPE_ELECTRIFY:
        case TOOL_SHAPE_BURNINATE:
        case TOOL_SHAPE_ICE:
        case TOOL_SHAPE_GAS:
        case TOOL_SHAPE_UNLETHAL:
        case TOOL_SHAPE_PLASMA:
        case TOOL_UNPHYSICS:
        case TOOL_SHAPE_VERTEX_EDIT:
        {
            return (type & E_TYPE_TOOL) != 0 && edit;
        }

        case TOOL_SHAPE_FLOOD_FILL:
        {
            return (type & (E_TYPE_TOOL | E_TYPE_PRIMITIVE_SHAPE)) != 0 && edit;
        }

        default: return false;
    }
}

typedef std::set<CResourceDescriptor<RPlan>, std::less<CResourceDescriptor<RPlan> >, STLBucketAlloc<CResourceDescriptor<RPlan> > > PlanDescriptorSet;
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

bool CustomItemMatch(CInventoryView* view, CInventoryItem* item, NetworkPlayerID* owner)
{
    if (view->Descriptor.Type == gEmotesCustomId)
    {
        return IsEmoteItem(item->Plan.GetGUID()) || IsPlayerCustomizationTool(item->Details.ToolType);
    }
    
    if (view->Descriptor.Type == gUsedItemsCustomId)
    {
        u32 tool = item->Details.ToolType;

        if (tool != TOOL_NOT_A_TOOL)
        {
            if (tool == TOOL_DELETE_COMMUNITY_OBJECTS || tool == TOOL_DELETE_COMMUNITY_STICKERS) return false;
            return true;
        }

        return gUsedPlanDescriptors.find(item->Plan) != gUsedPlanDescriptors.end();
    }

    CGUID item_guid = item->Plan.GetGUID();
    if (IsEmoteItem(item_guid)) return false;

    u32 item_type = item->Details.Type;
    u32 view_type = view->Descriptor.Type;

    u32 item_subtype = item->Details.SubType;
    u32 view_subtype = view->Descriptor.SubType;

    if (view->HeartedOnly && (item->Flags & E_IIF_HEARTED) == 0) return false;
    if (item->Details.ToolType != TOOL_NOT_A_TOOL) return IsToolMatch(view, item->Details.ToolType);

    if (item_guid == 0x12981 || item_guid == 0x15351) return false;
    if ((item_type & view_type) == 0) return false;

    if (view_type == gPodMask)
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

u32 DoInventorySoundObjectButton(CPoppetChild* gooey, u64 uid, CInventoryItem* item, v4 col, bool wide_icons, bool roundy_bg)
{
    CGooeyNodeManager* manager = *(CGooeyNodeManager**)((char*)gooey + 0x250);
    u32 res = 0;

    manager->DoBreak();
    if (manager->StartFrame())
    {
        manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
        manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
        manager->SetFrameHighlightStyle(GHS_ROUNDED_RECT);
        manager->SetFrameHighlightSizing((EGooeySizingType)648);
        if (manager->StartFrameNamed(uid))
        {
            manager->SetFrameLayoutMode(LM_CENTERED, LM_CENTERED);
            manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
            manager->SetFrameDefaultChildSpacing(64.0f, 0.0f);
            manager->SetFrameCornerRadius(64.0f);
            // manager->SetFrameOutlineWidth(48.0f);
            if (manager->StartFrame())
            {
                manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
                manager->SetFrameSizing(0.0f, 0.0f);

                /* 0xa140200 does a loading indicator, not sure which bit */

                CP<RTexture> icon = item->Details.Icon.GetRef();
                manager->DoImageButtonNamed(manager->GetAnonymousUID(), icon, v2(128.0f), v4(1.0f), 0x0);

                manager->EndFrame();
            }

            if (manager->StartFrame())
            {
                manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Contents());
                manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_CENTERED);
                manager->SetFrameBorders(8.0f, 8.0f);
                manager->SetFrameApplyClip(true, false);

                const tchar_t* text = item->Details.TranslateName();
                // if (!CustomTryTranslate(item->Details.NameTranslationTag, text))
                //     text = (const tchar_t*)L"";
                manager->DoTitle((wchar_t*)text, GTS_B1, v2(-1.0f, 0.0f));
                
                manager->EndFrame();
            }

            res = manager->EndFrame(256);
        }

        manager->EndFrame();
    }

    manager->DoBreak();

    return res;
}

struct BaseGroupBy {
    RLocalProfile* LocalProfile;
    CInventoryView* Page;
};

#include <algorithm>

struct SortByName
{
    bool operator()(CInventoryView::SInventoryItemData& a, CInventoryView::SInventoryItemData& z) const
    {
        const tchar_t* name_a = a.Item->Details.TranslateName();
        const tchar_t* name_b = z.Item->Details.TranslateName();

        int cmp = wcscmp((const wchar_t*)name_a, (const wchar_t*)name_b);
        if (cmp == 0) return a.Item->UID < z.Item->UID;

        return cmp < 0;
    }
};

struct SortByArtist
{
    bool operator()(CInventoryView::SInventoryItemData& a, CInventoryView::SInventoryItemData& z) const
    {
        if (a.Item->Details.CustomSortKey != z.Item->Details.CustomSortKey)
        {
            return a.Item->Details.CustomSortKey > z.Item->Details.CustomSortKey;
        }

        return a.Item->UID > z.Item->UID;
    }
};

void DoArtistSort(CInventoryView* view)
{
    std::sort(view->PageData.begin(), view->PageData.end(), SortByArtist());

    int key = 0xDEADBEEF;
    for (int i = 0; i < view->PageData.size(); ++i)
    {
        CInventoryItem* item = view->PageData[i].Item;
        if (item->Details.CustomSortKey != key)
        {
            SSortTermBoundary boundary;
            boundary.Index = i;
            MultiByteToWChar(boundary.Name, "No Artist", NULL);
            view->SortTermBoundaries.push_back(boundary);

            key = item->Details.CustomSortKey;
        }

    }
}

struct SortByPreference
{
    bool operator()(CInventoryView::SInventoryItemData& a, CInventoryView::SInventoryItemData& z) const
    {
        return a.Item->Details.NameTranslationTag == 1244778990;
    }
};

void DoPreferenceSort(CInventoryView* view)
{
    std::sort(view->PageData.begin(), view->PageData.end(), SortByPreference());

    SSortTermBoundary boundary;
    MultiByteToWChar(boundary.Name, "Good", NULL);
    view->SortTermBoundaries.push_back(boundary);

    boundary.Index = 1;
    MultiByteToWChar(boundary.Name, "Shit", NULL);
    view->SortTermBoundaries.push_back(boundary);
}

void SortBoundariesAlphabetically(CInventoryView* view)
{
    if (view->Descriptor.Type != E_TYPE_MUSIC) return;

    for (int i = 0; i < view->SortTermBoundaries.size(); ++i)
    {
        SSortTermBoundary& boundary = view->SortTermBoundaries[i];

        int end = view->PageData.size();
        if (i + 1 < view->SortTermBoundaries.size())
            end = view->SortTermBoundaries[i + 1].Index;

        std::sort(view->PageData.begin() + boundary.Index, view->PageData.begin() + end, SortByName());
    }
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

extern "C" uintptr_t _global_artist_hook;
extern "C" uintptr_t _global_pref_hook;
extern "C" uintptr_t _custom_tool_type_hook;
extern "C" uintptr_t _custom_poppet_message_hook;
extern "C" uintptr_t _custom_pick_object_action_hook;

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

void AttachCustomSortModes()
{
    MH_Poke32(0x000c2ca0, 0x2b990000 + (NUM_SORT_MODES - 1));

    // Initialise the switch table with the offsets to the invalid resource type case
    const int SWITCH_LABEL = 0x000c2cc4;
    const int NOP_LABEL = 0x000c2ce0;
    const int LABEL_COUNT = 0x7;
    static s32 TABLE[NUM_SORT_MODES];
    for (int i = 0; i < NUM_SORT_MODES; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    // Using the location sort for the artist sort
    TABLE[SORT_ARTIST] = (u32)&_global_artist_hook - (u32)TABLE;
    TABLE[SORT_MEME] = (u32)&_global_pref_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0091e06c, (u32)TABLE);
}

struct PageSection {
    SSortTermBoundary* Boundary;
    CRawVector<CInventoryItem*> ItemList;
};

struct SectionSettings {
    bool IsPlayerColourPage;
    u32 NumLocalPlayers;
    EGooeySizingType HighlightSizing;
    bool IsFirstSection;
};

u64 GetSectionUID(u64 page_uid, u32 section_id)
{
    return (u64)section_id * 10000ULL + page_uid;
}

u64 GetItemUID(u64 frame_id, u32 i)
{
    return (u64)i + frame_id + 1 | 0xbeefface00000000ull;
}

u64 GooeyIDFromScriptInt(int v)
{
    return (u64)v | 0xbeefface00000000ull;
}

struct EPoppetItemArgs {
    v2 size;
    u32 last_item_id;
    bool faded;
    bool is_costume_page;
    bool wide_icons;
};

static u32 gFunctionCategoryID = JenkinsHash((u8*)L"Functions", 18, 0);


MH_DefineFunc(CPoppetGooey_SetToolTip, 0x0037dd48, TOC1, void, CPoppetChild*, const wchar_t* tooltip, v4 rect, u64 uid, bool instant);
MH_DefineFunc(CPoppetGooey_ShowInfoBubble, 0x0037dd98, TOC1, void, CPoppetChild*, int mode);
MH_DefineFunc(CPoppetGooey_UpdateFocus, 0x0037dde0, TOC1, void, CPoppetChild*, u64 uid, bool);


MH_DefineFunc(DoPoppetItem, 0x0037ffb0, TOC1, void, CPoppetChild*, u64, CInventoryItem*, CInventoryCollection*, u32, const EPoppetItemArgs&, bool&, const CP<RLocalProfile>&);
MH_DefineFunc(DoSectionBreak, 0x00380c78, TOC1, void, CPoppetChild*, const tchar_t*, bool);
void CustomDoPoppetSection(
    CPoppetChild* gooey, PageSection* page_section, SectionSettings& settings, bool hide_section_titles,
    u64 frame_id, CInventoryCollection* current_cache, u32 page_number, EPoppetItemArgs item_args,
    u32& uid_offset, bool& first_section_break, bool& filler, const tchar_t* override_title
)
{
    if (page_section->ItemList.empty()) return;

    CPoppet* poppet = gooey->GetParent();
    const CP<RLocalProfile>& prf = poppet != NULL ? poppet->GetLocalProfile() : RLocalProfile::EMPTY;

    CGooeyNodeManager* manager = *(CGooeyNodeManager**)((char*)gooey + 0x250);
    u64 section_uid = GetSectionUID(frame_id, uid_offset++);

    if (!manager->StartFrame(GBS_NO_STYLE)) return;
    manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
    manager->SetFrameDefaultChildSpacing(16.0f, 16.0f);

    manager->SetFrameHighlightStyle(GHS_DROP_SHADOW);
    manager->SetFrameScrollStyle(GSS_DEFAULT, GSS_PROPORTIONAL);
    manager->SetFrameDrawScrollArrows(true, false, false, false);
    if (settings.IsFirstSection)
    {
        manager->SetFrameWhollyVisibleWithChildren();
        settings.IsFirstSection = false;
    }

    manager->SetFrameHighlightSizing(settings.HighlightSizing);

    if (settings.IsPlayerColourPage)
    {
        SizingBehaviour behaviour = SizingBehaviour::Relative(0.5f);
        if (settings.NumLocalPlayers < 2)
        {
            behaviour = SizingBehaviour::Relative(0.33f);
            manager->AddFrameColumn(behaviour, LM_CENTERED);   
        }

        manager->AddFrameColumn(behaviour, LM_CENTERED);
        manager->AddFrameColumn(behaviour, LM_CENTERED);
    }

    const tchar_t* title = override_title != NULL ? override_title : (const tchar_t*)page_section->Boundary->Name.c_str();
    u32 category_id = JenkinsHash((u8*)title, StringLength(title) * sizeof(tchar_t), 0);

    bool is_hidden = false;
    bool can_hide = category_id != gFunctionCategoryID;

    if (can_hide)
    {
        for (int i = 0; i < prf->HiddenCategories.size(); ++i)
        {
            if (prf->HiddenCategories[i] == category_id)
            {
                is_hidden = true;
                break;
            }
        }
    }
    
    bool backgrounds = current_cache->InventoryViews[page_number]->Descriptor.Type == E_TYPE_BACKGROUND;
    bool sound_objects = current_cache->InventoryViews[page_number]->Descriptor.Type == E_TYPE_SOUND;

    if (!settings.IsPlayerColourPage && !backgrounds && !sound_objects)
    // if (!hide_section_titles)
    {
        // float height = is_hidden ? 2.0f : 24.0f;
        float height = 24.0f;
        manager->SetFrameBorders(0.0f, height, 0.0f, height);
        manager->DoBreak();
        if (manager->StartFrame(GBS_NO_STYLE))
        {
            manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_CENTERED);
            manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
            manager->SetFrameBorders(0.0f, 0.0f, 0.0f, 16.0f);

            u64 item_uid = GetItemUID(section_uid, 0);
            u32 result = manager->DoFancyButtonNamed(item_uid, (wchar_t*)title, GTS_T5, GBS_LABEL, STATE_NORMAL, can_hide ? 0x100 : 0x0, NULL);
            if (result & 0x100)
            {
                if (is_hidden)
                {
                    for (u32* it = prf->HiddenCategories.begin(); it != prf->HiddenCategories.end(); ++it)
                    {
                        if (*it == category_id)
                        {
                            prf->HiddenCategories.erase(it);
                            break;
                        }
                    }
                }
                else
                {
                    prf->HiddenCategories.push_back(category_id);
                }

                is_hidden = !is_hidden;
            }

            if (result & 0x400000)
            {
                CRect rect = manager->GetLastItemScreenRect();
                CPoppetGooey_SetToolTip(gooey, L"", v4(rect.tl.getX(), rect.tl.getY(), rect.br.getX(), rect.br.getY()), item_uid, false);
            }

            manager->DoBreak(GBS_DASHED, v2(1.2f, 0.5f), 20.0f);
            manager->EndFrame();
        }

        manager->DoBreak();
    }

    first_section_break = false;

    if (!is_hidden)
    {
        for (int i = 0; i < page_section->ItemList.size(); ++i)
        {
            CInventoryItem* item = page_section->ItemList[i];
            u64 item_uid = GetItemUID(section_uid, i + 1);
            DoPoppetItem(gooey, item_uid, item, current_cache, page_number, item_args, filler, prf);
        }
    }

    manager->EndFrame();
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
    MH_PokeBranch(0x0038019c, &_custom_item_grid_hook);

    // Allow fevs in audiophiles rlst
    MH_PokeBranch(0x001a4a98, &_custom_event_projects_hook);

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
}