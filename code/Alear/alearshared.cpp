#include "alearshared.h"
#include "alearoptui.h"
#include "alearcam.h"
#include "alearconf.h"
#include "pins.h"
#include "outfits.h"

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
#include <LoadingScreen.h>
#include <InventoryView.h>
#include <InventoryItem.h>
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

enum EItemRequestResult {
    E_ITEM_IDLE,
    E_ITEM_PENDING,

    E_ITEM_NOT_FOUND,
    E_ITEM_NO_PLAYER,
    E_ITEM_ADDED,
    E_ITEM_ALREADY_EXISTS,
    E_ITEM_FAILED_LOAD,
    E_ITEM_NO_DATA_SOURCE,
    E_ITEM_ALREADY_IN_PROGRESS
};

class CItemRequest {
public:
    EItemRequestResult Request(const void* data, u32 len)
    {
        if (HasRequest()) return E_ITEM_ALREADY_IN_PROGRESS;
        if (len == 0) return E_ITEM_NO_DATA_SOURCE;

        CHash hash;
        if (!SaveFileDataToCache(CT_TEMP, data, len,  hash)) return E_ITEM_NO_DATA_SOURCE;

        CResourceDescriptor<RPlan> desc(hash);
        Plan = LoadResource<RPlan>(desc, 0, STREAM_PRIORITY_DEFAULT, false);

        Result = E_ITEM_PENDING;
        while (Result == E_ITEM_PENDING) ThreadSleep(10);

        EItemRequestResult result = Result;
        Destroy();
        return result;
    }

    EItemRequestResult Request(const char* filename)
    {
        if (HasRequest()) return E_ITEM_ALREADY_IN_PROGRESS;

        CFileDBRow* row = FileDB::FindByPath(filename, false);
        if (row == NULL) return E_ITEM_NOT_FOUND;

        Plan = LoadResourceByKey<RPlan>(row->FileGuid.guid, 0, STREAM_PRIORITY_DEFAULT);

        Result = E_ITEM_PENDING;

        while (Result == E_ITEM_PENDING) ThreadSleep(10);

        EItemRequestResult result = Result;
        Destroy();
        return result;
    }

    inline const CP<RPlan>& GetPlan() const { return Plan; }
    inline bool HasRequest() const { return Result == E_ITEM_PENDING; }

    inline void SetResult(EItemRequestResult result)
    {
        Result = result;
    }
private:
    inline void Destroy()
    {
        Plan = NULL;
        Result = E_ITEM_IDLE;
    }
private:
    CP<RPlan> Plan;
    volatile EItemRequestResult Result;
};

CItemRequest gItemRequest;

void UpdateItemRequest()
{
    if (!gItemRequest.HasRequest()) return;

    const CP<RPlan>& plan = gItemRequest.GetPlan();

    CThing* player_thing = gGame->GetYellowheadFromPlayerNumber(gNetworkManager.InputManager.GetLocalLeadersPlayerNumber());
    if (player_thing == NULL)
    {
        gItemRequest.SetResult(E_ITEM_NO_PLAYER);
        return;
    }

    plan->BlockUntilLoaded();
    if (plan->IsError())
    {
        gItemRequest.SetResult(E_ITEM_FAILED_LOAD);
        return;
    }

    PYellowHead* yellowhead = player_thing->GetPYellowHead();

    if (yellowhead->GetLocalProfile()->AddInventoryItem(plan, 0, 0, false, false, NULL))
    {
        SpawnCollectBubble(player_thing, plan);
        gItemRequest.SetResult(E_ITEM_ADDED);
        return;
    }

    gItemRequest.SetResult(E_ITEM_ALREADY_EXISTS);
}

#include "alearsync.h"

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

        CThing* hover = poppet->Edit.LastHoverThing;
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
    //     CP<RPlan> plan = LoadResourceByKey<RPlan>(31704, 0, STREAM_PRIORITY_DEFAULT);
    //     plan->BlockUntilLoaded();
    //     PYellowHead* yellowhead = world->ListPYellowHead[0];
    //     CThing* player_thing = yellowhead->GetThing();
    //     SpawnCollectBubble(player_thing, plan);
    // }



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

        case TOOL_SHAPE_ELECTRIFY:
        case TOOL_SHAPE_BURNINATE:
        case TOOL_SHAPE_ICE:
        case TOOL_SHAPE_GAS:
        case TOOL_SHAPE_UNLETHAL:
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

bool CustomItemMatch(CInventoryView* view, CInventoryItem* item, NetworkPlayerID* owner)
{
    CGUID item_guid = item->Plan.GetGUID();

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

struct BaseGroupBy {
    RLocalProfile* LocalProfile;
    CInventoryView* Page;
};

#include <algorithm>

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

extern "C" uintptr_t _global_artist_hook();
extern "C" uintptr_t _global_pref_hook();

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

class CInventoryCollection : public CBaseCounted, public CReflectionVisitable {
public:
    CVector<CP<CInventoryView> > InventoryViews;
    NetworkPlayerID PlayerID;
    u32 CurrentPageNumber;
    u32 CollectionID;
    u32 ActionOnItemSelect;
    bool MenuFunctionalityLimited;
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

    if (!settings.IsPlayerColourPage && !backgrounds)
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


class MMOTextStreamA {
public:
    struct AsHex {
        AsHex(u32 value) { Value = value; }
        u32 Value;
    };

    struct FmtInt {
        FmtInt(const char* format, u32 value) { Format = format; Value = value; }
        const char* Format;
        u32 Value;
    };
public:
    virtual ~MMOTextStreamA() = 0;
public:
    inline MMOTextStreamA& operator<<(int v)
    {
        char fmt[64];
        FormatString<64>(fmt, "%d", v);
        return *this << fmt;
    }

    inline MMOTextStreamA& operator<<(v4 v)
    {
        char fmt[256];
        FormatString<256>(fmt, "[%f, %f, %f]", v.getX().getAsFloat(), v.getY().getAsFloat(), v.getZ().getAsFloat());
        return *this << fmt;
    }

    inline MMOTextStreamA& operator<<(char const* s)
    {
        OutputString(s);
        return *this;
    }

    inline MMOTextStreamA& operator<<(MMString<char>& s)
    {
        OutputString(s.c_str());
        return *this;
    }
public:
    virtual void OutputData(const void* data, u32 len) = 0;
private:
    virtual void OutputString(const char* s) = 0;
    virtual void OutputString(const wchar_t* s) = 0;
    virtual void OutputString(const tchar_t* s) = 0;
};

struct SCompareIgnoreCase {
	bool operator()(const MMString<char>& a, const MMString<char>& b) const
	{
		return StringICompare(a.c_str(), b.c_str());
	}
};

typedef std::map<MMString<char>, MMString<char>, SCompareIgnoreCase, STLBucketAlloc<MMString<char> > > ParameterMap;

class CRoute;
class CWebternate {
public:
    virtual ~CWebternate() = 0;
public:
    virtual void AddRoute(CRoute* page) = 0;
    virtual void Unknown_1() = 0;
    virtual int GetPageCount() const = 0;
    virtual CRoute* GetPage(int i) const;
};
class CRoute {
public:
    inline CRoute(CWebternate* owner) : Webternate(owner) {}
    virtual ~CRoute() {}
public:
    virtual MMString<char> GetFileName(ParameterMap& parameters) 
    {
        return "\0";
    };
    
    virtual const char* GetHref() { return NULL; }
    virtual const char* GetContentType() 
    { 
        return "text/html; charset=UTF-8"; 
    }
    virtual int GetContentLength() { return -1; }
    virtual void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body) {};
    virtual bool IsPage() { return true; }
    virtual bool Unknown_8() { return true; }
    virtual const char* GetTitle() { return NULL; }
    virtual int GetRefreshInterval() { return -1; }

    virtual void DoNavigation(MMOTextStreamA& stream) 
    {
        // stream << "<ul>";
        // for (int i = 0; i < Webternate->GetPageCount(); ++i)
        // {
        //     CRoute* page = Webternate->GetPage(i);
        //     if (!page->IsPage() || !page->Unknown_8()) continue;
        //     stream << "<li><a href=\"" << page->GetHref() << "\">";
        //     stream << page->GetTitle();
        //     stream << "</a></li>";
        // }
        // stream << "</ul>";

        stream << "<table><tr>";
        for (int i = 0; i < Webternate->GetPageCount(); ++i)
        {
            CRoute* page = Webternate->GetPage(i);
            if (!page->IsPage() || !page->Unknown_8()) continue;
            stream << "<td><a href=\"" << page->GetHref() << "\">";
            stream << page->GetTitle();
            stream << "</a></td>";
        }
        stream << "</tr></table><br>";
    }

    virtual void DoHeader(MMOTextStreamA& stream) 
    {
        stream << "<!DOCTYPE html><html lang=\"en-US\"><head>";
        if (GetRefreshInterval() != -1)
        {
            stream << "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"";
            stream << GetRefreshInterval();
            stream << ";URL=";
            stream << GetHref();
            stream << "\">";
        }

        stream << "<meta charset=\"utf-8\" />";
        stream << "<title>" << GetTitle() << "</title>";
        stream << "<link href=\"/r?path=gamedata/alear/web/base.css\" rel=\"stylesheet\" type=\"text/css\">";

        stream << "<body>";
        DoNavigation(stream);
    }

    virtual void Unknown_13() {}

    virtual void DoFooter(MMOTextStreamA& stream) 
    {
        stream << "<p><a href=\"/\">Index</a></body></html>\n";
    }
protected:
    void DoFileContents(MMOTextStreamA& stream, const char* path)
    {
        CFilePath fp(path);
        CFileDBRow* row = FileDB::FindByPath(fp, false);
        if (row == NULL) return;

        ByteArray b;
        if (!GetFileDataFromCaches(row->FileHash, b))
        {
            CHash hash;
            fp.Assign(FPR_GAMEDATA, row->FilePathX);
            FileLoad(fp, b, hash);
        }

        if (b.empty()) return;
        stream.OutputData((const void*)b.begin(), b.size());
    }
private:
    CWebternate* Webternate;
};

class CFaviconEndpoint : public CRoute {
public:
    inline CFaviconEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "favicon.ico"; }
    const char* GetContentType() { return "image/x-icon"; }
    bool IsPage() { return false; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        DoFileContents(stream, "gamedata/alear/web/favicon.ico");
    }
};

class CLookupResourceEndpoint : public CRoute {
public:
    inline CLookupResourceEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "r"; }

    MMString<char> GetFileName(ParameterMap& parameters) 
    {
        // webternate is single threaded, so its fine to do this
        CachedContentType = "application/octet-stream";

        typename ParameterMap::iterator it = parameters.find("path");
        if (it != parameters.end())
        {
            const char* path = it->second.c_str();

            if (strstr(path, ".css") != NULL) CachedContentType = "text/css";
            else if (strstr(path, ".json") != NULL) CachedContentType = "application/json";
            else if (strstr(path, ".js") != NULL) CachedContentType = "text/javascript";
            else if (strstr(path, ".html") != NULL) CachedContentType = "text/html; charset=UTF-8";
            // only really need to return the filename if we're using binary content, i think?
            else return path;
        }

        return "\0";
    };

    const char* GetContentType() { return CachedContentType; }
    bool IsPage() { return false; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        typename ParameterMap::iterator it = parameters.find("path");
        if (it != parameters.end())
            DoFileContents(stream, it->second.c_str());
    }
private:
    const char* CachedContentType;
};

class CInventoryEndpoint : public CRoute {
public:
    inline CInventoryEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "api/inventory"; }
    const char* GetContentType() { return "application/json"; }
    bool IsPage() { return false; }

    const char* GetItemResultCode(EItemRequestResult result)
    {
        switch (result)
        {
            case E_ITEM_NOT_FOUND: 
                return "itemNotFound";
            case E_ITEM_NO_PLAYER: 
                return "yellowheadNotFound";
            case E_ITEM_ADDED: 
                return "itemAdded"; 
            case E_ITEM_ALREADY_EXISTS: 
                return "itemAlreadyExists"; 
            case E_ITEM_FAILED_LOAD: 
                return "itemLoadFail"; 
            case E_ITEM_ALREADY_IN_PROGRESS:
                return "itemRequestPending"; 
            case E_ITEM_NO_DATA_SOURCE:
                return "noDataSource"; 
            default: 
                return "invalidState"; 
        }
    }

    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        if (method == E_HTTP_GET) 
        {
            stream << "{\"result\":\"invalidMethod\"}";
            return;
        }

        char json[256];
        EItemRequestResult result;

        typename ParameterMap::iterator it = parameters.find("path");
        if (it != parameters.end())
        {
            result = gItemRequest.Request(it->second.c_str());
        }
        else
        {
            result = gItemRequest.Request((const void*)body.Begin, body.Length());
        }

        const char* code = GetItemResultCode(result);
        FormatString<256>(json, "{\"result\":\"%s\"}", code);
        stream << json;
    }
};

class CCinemachineEndpoint : public CRoute {
public:
    inline CCinemachineEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "api/cinemachine"; }
    const char* GetContentType() { return "application/json"; }
    bool IsPage() { return false; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        
        if (method == E_HTTP_GET)
        {
            stream << "{";
                stream << "\"pos\":" << GetCameraPosition() << ",";
                stream << "\"foc\":" << GetCameraFocus();
            stream << "}";

            return;
        }
    }
};

class CAurienPage : public CRoute {
public:
    inline CAurienPage(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "aurien"; }
    const char* GetTitle() { return "Old Man Requests"; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        if (method == E_HTTP_POST)
        {
            typename ParameterMap::iterator it = parameters.find("action");
            if (it != parameters.end())
            {
                const char* action = it->second.c_str();
                if (StringICompare(action, "record") == 0)
                {
                    if (gRecordingFileHandle != -1)
                        FileClose(&gRecordingFileHandle);
                    gDoRecording = !gDoRecording;
                }
            }

            return;
        }

        DoHeader(stream);
        DoFileContents(stream, "gamedata/alear/web/aurien.html");
        DoFooter(stream);

        return;
    }
};

class CInventoryPage : public CRoute {
public:
    inline CInventoryPage(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "inventory"; }
    const char* GetTitle() { return "Inventory"; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        // dumb hack, the range will always be set if we're in a POST request
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        if (method == E_HTTP_POST) return;

        DoHeader(stream);
        DoFileContents(stream, "gamedata/alear/web/inventory.html");
        DoFooter(stream);
    }
};

void OnWebternateSetup(CWebternate* webternate)
{
    webternate->AddRoute(new CInventoryPage(webternate));
    webternate->AddRoute(new CAurienPage(webternate));
    webternate->AddRoute(new CLookupResourceEndpoint(webternate));
    webternate->AddRoute(new CCinemachineEndpoint(webternate));
    webternate->AddRoute(new CInventoryEndpoint(webternate));
    webternate->AddRoute(new CFaviconEndpoint(webternate));
}

CP<CResource> TextureFromResource(CP<CResource>& res)
{
    if (!res) return NULL;
    EResourceType type = res->GetResourceType();
    if (type == RTYPE_TEXTURE || type == RTYPE_ANIMATED_TEXTURE) return res;
    return NULL;
}

extern "C" uintptr_t _gsub_rlst_hook;
extern "C" uintptr_t _popit_isitemselected_hook;
extern "C" uintptr_t _popit_dopoppetsection_hook;
extern "C" uintptr_t _global_onswapbuffers_hook;
extern "C" uintptr_t _initextradata_cthing_hook;
extern "C" uintptr_t _destroyextradata_cthing_hook;
extern "C" uintptr_t _global_webternate_hook;

extern "C" uintptr_t _create_boundary_artist_hook;
extern "C" uintptr_t _create_boundary_primary_index_artist_hook;

extern "C" uintptr_t _create_first_boundary_artist_hook;

extern "C" uintptr_t _apply_boundaries_sort_index_first;
extern "C" uintptr_t _apply_boundaries_sort_index_loop;

extern "C" uintptr_t _global_icon_size_hook;

extern "C" uintptr_t _gooey_image_ctor_animated_hook;
extern "C" uintptr_t _sdf_button_animated_hook;
extern "C" uintptr_t _gooey_image_update_hook;

void InitSharedHooks()
{
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

    // Hooks for animated texture buttons
    // MH_Poke32(0x006ae1ac, LI(4, sizeof(CGooeyImage)));
    // MH_PokeBranch(0x003227d8, &_gooey_image_ctor_animated_hook);
    // MH_PokeBranch(0x00300268, &_sdf_button_animated_hook);
    // MH_PokeBranch(0x00322f94, &_gooey_image_update_hook);
    // MH_InitHook((void*)0x0043be2c, (void*)&TextureFromResource);
}