#include "alearshared.h"
#include "alearoptui.h"
#include "alearlauncher.h"
#include "alearcam.h"
#include "alearconf.h"
#include "pins.h"

#include <cell/fs/cell_fs_file_api.h>
#include <cell/gcm.h>

#include <hook.h>
#include <printf.h>
#include <gfxcore.h>
#include <padinput.h>
#include <filepath.h>
#include <cell/DebugLog.h>

#include <thing.h>
#include <Mesh.h>
#include <Poppet.h>
#include <OverlayUI.h>
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
#include <ResourceTranslationTable.h>
#include <ResourceFileOfBytes.h>
#include <ResourceSystem.h>

#include <gooey/GooeyNodeManager.h>
#include <network/NetworkManager.h>

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

    float* vertices = (float*)((char*)mesh->SourceGeometry.CachedAddress + mesh->SourceStreamOffsets[0]);
    float* uv = ((float*)mesh->AttributeData.CachedAddress);

    u16* indices = (u16*)(mesh->Indices.CachedAddress);
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

void OnUpdateLevel()
{
    // Tick Mario's in the world if enabled.
    #ifdef __SM64__
    UpdateMarioAvatars();
    #endif

    // Handle any reloads for databases and caches,
    // if anything was changed on disk or requested from
    // a local network.
    ReloadPendingDatabases();

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

}

void OnRunPipelinePostProcessing()
{
    #if __SM64__
    UpdateMarioDebugRender();
    #endif 

    if (gShowOutlines) RenderShapeOutlines();
}

void OnPredictionOrRenderUpdate()
{
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
bool CustomItemMatch(CInventoryView* view, CInventoryItem* item, NetworkPlayerID* owner)
{
    CGUID item_guid = item->Plan.GetGUID();

    u32 item_type = item->Details.Type;
    u32 view_type = view->Descriptor.Type;

    u32 item_subtype = item->Details.SubType;
    u32 view_subtype = view->Descriptor.SubType;

    if (view->HeartedOnly && (item->Flags & E_IIF_HEARTED) == 0) return false;
    
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

extern "C" uintptr_t _gsub_rlst_hook;
void InitSharedHooks()
{
    MH_InitHook((void*)0x002eeb90, (void*)&CustomItemMatch);
    MH_InitHook((void*)0x000232bc, (void*)&CustomTryTranslate);
    MH_InitHook((void*)0x0009c52c, (void*)&CustomDoGUIDSubstitution);
    MH_PokeBranch(0x000b94e4, &_gsub_rlst_hook);
}