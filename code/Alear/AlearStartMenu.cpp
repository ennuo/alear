#include "AlearStartMenu.h"
#include "AlearSync.h"
#include "AlearShared.h"
#include "AlearConfig.h"
#include "ServerSwitcher.h"
#include "customization/Styles.h"
#include "customization/SlapStyles.h"
#include "FileWatcher.h"

#include <printf.h>
#include <hook.h>
#include <mmalex.h>
#include <ppcasm.h>
#include <filepath.h>
#include <printf.h>
#include <filepath.h>
#include <refcount.h>
#include <GuidHashMap.h>
#include <Hash.h>
#include <mem_stl_buckets.h>

#include <cell/fs/cell_fs_file_api.h>
#include <cell/DebugLog.h>

#include <gooey/GooeyNodeManager.h>
#include <gooey/GooeyScriptInterface.h>

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

#include <thing.h>
#include <PartPhysicsWorld.h>
#include <FartRO.h>
#include <PartScript.h>
#include <Resource.h>
#include <ResourceLevel.h>
#include <ResourceSystem.h>
#include <ResourceGame.h>
#include <ResourceScript.h>
#include <PartYellowHead.h>
#include <Poppet.h>
#include <CritSec.h>
#include <ResourceGFXTexture.h>
#include <padinput.h>
#include <GameShell.h>
#include <vm/VirtualMachine.h>

#include <InventoryItem.h>
#include <poppet/ScriptObjectPoppet.h>
#include <ResourceLocalProfile.h>


#ifdef __SM64__
extern void UpdateMarioAvatars();
extern void ClearMarioAvatars();
#endif

CVector<CFilePath> gReloadMap;
CCriticalSec gReloadCS("ReloadCS");
CVector<CFilePath> gPublishDatabases;

CSignature gForceMenuUpdateSig("ForceMenuUpdate__");
CSignature gSetSubPageSig("SetSubPage__i");

const u64 FIRST_SELECTION_UID = 0x414c525300000000ull;
const u64 PRIMARY_FRAME_UID = 0x414c52537fffffffull;
const u64 FIRST_SECONDARY_SELECTION_UID = 0x414c525300000000ull + 0x80000000ull;

CFolderNode gRecyclingBin("Recycling Bin");
CFolderNode gRootFolder("Filesystem");
CFolderNode* gSelectedFolder = NULL;
bool gShowRecycleBin;

CThing* GetStartMenuThing()
{
    COverlayUI* overlay = gGameShell->OverlayUI;
    if (!overlay || !overlay->IsStartMenuOpen()) return NULL;
    CP<RLevel>& level = overlay->HUDLevel;
    PWorld* world = level->GetWorld();
    if (world == NULL) return NULL;
    return world->GetThingByUID(17);
}

void ForceStartMenuUpdate()
{
    DebugLog("trying to force start menu update...");

    CThing* start_menu = GetStartMenuThing();
    if (start_menu == NULL)
    {
        DebugLog("start menu thing was null!\n");
        return;
    }

    PScript* script = start_menu->GetPScript();
    if (script == NULL)
    {
        DebugLog("part script was null!\n");
        return;
    }

    DebugLog("trying to invoke...\n");

    CScriptArguments arguments;
    if (!script->InvokeSync(gForceMenuUpdateSig, arguments))
        DebugLog("invoke failed!!\n");
}

void SetStartMenuSubPage(int page)
{
    CThing* start_menu = GetStartMenuThing();
    if (start_menu == NULL) return;
    PScript* script = start_menu->GetPScript();
    if (script == NULL) return;
    
    CScriptArguments arguments;
    CScriptVariant arg1(page);
    arguments.AppendArg(arg1);

    script->InvokeSync(gSetSubPageSig, arguments);
}

CBaseNode::~CBaseNode()
{

}

CBaseNode::CBaseNode(const char* name, ENodeType type) : Name()
{
    NameHash = JenkinsHash((u8*)name, StringLength(name), 0);
    MultiByteToWChar(Name, name, NULL);
    Type = type;
    Parent = NULL;
    InRecyclingBin = false;
}

void CBaseNode::ToggleDelete()
{
    if (InRecyclingBin)
    {
        for (CBaseNode** it = gRecyclingBin.Files.begin(); it != gRecyclingBin.Files.end(); ++it)
        {
            if (*it == this)
            {
                gRecyclingBin.Files.erase(it);
                break;
            }
        }

        InRecyclingBin = false;
        return;
    }

    gRecyclingBin.Files.push_back(this);
    InRecyclingBin = true;
}

CFolderNode::CFolderNode(const char* name) : CBaseNode(name, E_FOLDER), Folders(), Files()
{
    LastSelectedUID = FIRST_SELECTION_UID;
}

CFolderNode::~CFolderNode()
{
    DestroyHierarchy();
}

void CFolderNode::DestroyHierarchy()
{
    for (CFolderNode** folder = Folders.begin(); folder != Folders.end(); ++folder)
        delete (*folder);
    for (CBaseNode** file = Files.begin(); file != Files.end(); ++file)
        delete (*file);

    Folders.clear();
    Files.clear();
}

CFolderNode* CFolderNode::AddFolder(const char* name)
{
    int hash = JenkinsHash((u8*)name, StringLength(name), 0);
    for (int i = 0; i < Folders.size(); ++i)
    {
        CFolderNode* folder = Folders[i];
        if (folder->GetNameHash() == hash)
            return folder;
    }

    CFolderNode* folder = new CFolderNode(name);
    folder->Parent = this;
    Folders.push_back(folder);

    return folder;
}

CBaseNode* CFolderNode::AddFile(const char* name, CGUID guid)
{
    int hash = JenkinsHash((u8*)name, StringLength(name), 0);
    for (int i = 0; i < Files.size(); ++i)
    {
        CBaseNode* file = Files[i];
        if (file->GetNameHash() == hash)
            return file;
    }

    CFileNode* file = new CFileNode(name, guid);
    file->Parent = this;
    Files.push_back(file);
    
    return file;
}

bool gPopAlearSubPage;

void ReloadSyncFilesystem()
{
    gRootFolder.DestroyHierarchy();
    gRecyclingBin.Files.clear();
    gShowRecycleBin = false;
    gSelectedFolder = &gRootFolder;
    gSelectedFolder->LastSelectedUID = FIRST_SELECTION_UID;

    CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
    CFileDB* syncdb = FileDB::DBs[0];
    if (syncdb == NULL || syncdb->Files.size() == 0) return;

    char name[512];
    for (CFileDBRow* row = syncdb->Files.begin(); row != syncdb->Files.end(); ++row)
    {
        CFolderNode* node = &gRootFolder;

        char* path = row->FilePathX;
        char* index = strchr(path, '/');
        while (index != NULL)
        {
            int len = index - path;
            strncpy(name, path, len);
            name[len] = '\0';

            node = node->AddFolder(name);

            path = index + 1;
            index = strchr(path, '/');
        }

        node->AddFile(path, row->FileGuid);
    }
}

volatile u32 gStartMenuNotification;
void NotifyStartMenu(u32 flags)
{
    gStartMenuNotification |= flags;
}

void ProcessStartMenuNotifications()
{
    u32 flags = gStartMenuNotification;
    if (flags == 0) return;

    if (gGameShell->OverlayUI->IsStartMenuOpen())
    {
        if (flags & NOTIFICATION_RELOAD_FILESYSTEM)
            ReloadSyncFilesystem();
        if (flags & NOTIFICATION_REFRESH_UI)
            ForceStartMenuUpdate();
    }

    gStartMenuNotification = 0;
}

void ReloadPublishDatabases()
{
    gPublishDatabases.clear();

    CFilePath publishdir(FPR_GAMEDATA, gSyncPublishPath);

    int fd;
    if (cellFsOpendir(publishdir.c_str(), &fd) != CELL_FS_OK) return;

    CellFsDirectoryEntry entry;
    memset(&entry, 0, sizeof(CellFsDirectoryEntry));
    u32 data_count = 0;
    char buf[CELL_FS_MAX_FS_PATH_LENGTH];
    CFilePath fp;
    do
    {
        if (cellFsGetDirectoryEntries(fd, &entry, sizeof(CellFsDirectoryEntry), &data_count) != CELL_FS_OK)
            break;
        
        if (entry.entry_name.d_type != CELL_FS_TYPE_REGULAR) continue;
        if (strstr(entry.entry_name.d_name, ".map") == NULL) continue;

        sprintf(buf, "%s/%s", publishdir.c_str(), entry.entry_name.d_name);
        fp.Assign(buf);

        gPublishDatabases.push_back(fp);

    } while (data_count);

    cellFsClosedir(fd);

}

void ReloadReadonlyCaches()
{
    CCSLock _res_lock(gResourceCS, __FILE__, __LINE__);
    CFartManyRO* caches = (CFartManyRO*)gCaches[CT_READONLY];
    DebugLog("CFartManyRO is being relinked! Current statistics: FAT=[%d], FART=[%d]\n", caches->FAT.size(), caches->Farts.size());
    if (caches == NULL) return;

    // override operator= though?
    // dont know if the official one is meant to be a
    // shallow or deep copy though
    CVector<CFartRO*> farts(caches->Farts.size());
    for (int i = 0; i < caches->Farts.size(); ++i)
        farts.push_back(caches->Farts[i]);
    
    caches->FAT.clear();
    caches->Farts.clear();
    
    CFartRO** iter = farts.begin();
    for (; iter != farts.end(); ++iter)
    {
        CFartRO* fart = *iter;

        fart->CloseCache(false);
        if (!fart->OpenCache())
            fart->CloseCache(false);

        IncorporateFartRO(fart);
    }

    DebugLog("CFartManyRO has been relinked! FAT=[%d], FART=[%d]\n", caches->FAT.size(), caches->Farts.size());
}


struct RelinkTarget {
    RelinkTarget() : Row(), Resource() {}
    RelinkTarget(CFileDBRow* row, CResource* resource)
    {
        Row = row;
        Resource = resource;
    }

    CFileDBRow* Row;
    CResource* Resource;
};

void ReloadModifiedResources()
{
    ReloadModifiedResources(NULL, NULL);
}

void ReloadModifiedResources(CFileDB* database, CFileDB* old_database)
{
    CVector<RelinkTarget> relinker;
    CVector<CResource*> scripts;

    bool relink_scripts = false;

    {
        CCSLock _the_lock(gResourceCS, __FILE__, __LINE__);
        for (int i = 0; i < gResourceArray.size(); ++i)
        {
            CResource* resource = gResourceArray[i];
            EResourceType type = resource->GetResourceType();
            CGUID guid = resource->GetGUID();
            if (guid == 0) continue;
            
            // If we have a filter database supplied, ignore any resources not contained
            if (database != NULL && database->FindByGUID(guid) == NULL)
            {
                if (old_database == NULL || old_database->FindByGUID(guid) == NULL)
                    continue;
            }

            if (type == RTYPE_SPU_ELF) continue;

            CFileDBRow* row = FileDB::FindByGUID(guid);
            if (row == NULL) continue;
            
            CHash& loaded_hash = resource->GetLoadedHash();
            CHash& file_hash = row->FileHash;
            if (!loaded_hash || file_hash != loaded_hash)
            {
                if (type == RTYPE_SCRIPT) 
                {
                    relink_scripts = true;
                    scripts.push_back(resource);
                }

                relinker.push_back(RelinkTarget(row, resource));

                // Reloading the file doesn't seem to change the loaded hash?
                // So just forcibly reset it here.
                loaded_hash = file_hash;
            }
        }
    }

    char hash[HASH_HEX_STRING_LENGTH];
    DebugLog("Reloading %d GUIDs:\n", relinker.size());
    for (int i = 0; i < relinker.size(); ++i)
    {
        RelinkTarget& target = relinker[i];
        CFileDBRow* row = target.Row;
        CResource* resource = target.Resource;

        row->FileHash.ConvertToHex(hash);
        DebugLog("\tg0x%08x (h%s) -> (%s)\n", row->FileGuid.guid, hash, row->FilePathX);
        
        UnloadResource(resource);
        LoadResource(resource, STREAM_PRIORITY_DEFAULT);

        if (row->FileGuid.guid == E_SLAP_STYLES_KEY) LoadSlapStyles();
    }

    _BlockUntilAllResourcesLoaded(DONT_WAIT_FOR_STREAMING_RESOURCES);

    if (relink_scripts)
    {
        for (CResource** iter = scripts.begin(); iter != scripts.end(); ++iter)
        {
            RScript* script = (RScript*)*iter;
            if (!script->IsInstanceLayoutValid())
                script->ForceFixup();
            script->Fixup();
        }

        CRawVector<CResource*> levels;
        GetResourceSet(levels, RTYPE_LEVEL);
        for (CResource** iter = levels.begin(); iter != levels.end(); ++iter)
        {
            RLevel* level = (RLevel*)*iter;
            CThing* thing = level->WorldThing;
            if (thing == NULL) continue;
            PWorld* world = thing->GetPWorld();
            if (world == NULL) continue;
            world->UpgradeAllScripts();
        }


    }
}

void PrintLoadedResources()
{
    CCSLock _the_lock(gResourceCS, __FILE__, __LINE__);
    char hash[HASH_HEX_STRING_LENGTH];

    DebugLog("Printing resource list...\n");
    for (int i = 0; i < gResourceArray.size(); ++i)
    {
        CResource* resource = gResourceArray[i];
        CGUID guid = resource->GetGUID();
        if (guid == 0) continue;

        CFileDBRow* row = FileDB::FindByGUID(guid);

        if (row == NULL) continue;
        row->FileHash.ConvertToHex(hash);
        DebugLog("\tg0x%08x (h%s) -> %s (0x%08x)\n", row->FileGuid.guid, hash, row->FilePathX, resource);
    }
}

void GetRelativePath(MMString<wchar_t>& wstr, CFilePath& fp)
{
    const char* c_str = fp.c_str();
    const char* path = strstr(c_str, "USRDIR/");
    if (path == NULL) path = c_str;
    else path += 7;

    MultiByteToWChar(wstr, path, NULL);
}

void DoSectionHeader(CGooeyNodeManager* manager, wchar_t* title)
{
    manager->DoBreak();
    manager->DoSpacer();
    manager->DoBreak();
    manager->DoText(title, GTS_T2);
    manager->DoHorizontalBreak(GBS_SOLID, v2(0.5f, 0.5f));
}

void DoGamedataSubmenu(CGooeyNodeManager* manager)
{
    MMString<wchar_t> wstr;
    DoSectionHeader(manager, L"Loaded Caches");
    
    CFartManyRO* caches = (CFartManyRO*)gCaches[CT_READONLY]; 
    if (caches != NULL)
    {
        for (int i = 0; i < caches->Farts.size(); ++i)
        {
            CFartRO* cache = caches->Farts[i];
            GetRelativePath(wstr, cache->fp);
            manager->DoInline(wstr.c_str(), GTS_T5, STATE_NORMAL, NULL, 0);
            manager->DoBreak();
        }
    }

    DoSectionHeader(manager, L"Loaded Databases");
    {
        CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
        for (int i = 0; i < FileDB::DBs.size(); ++i)
        {
            CFileDB* database = FileDB::DBs[i];
            CFilePath fp = database->Path;
            GetRelativePath(wstr, fp);
            u32 ret = manager->DoInline(wstr.c_str(), GTS_T5, STATE_NORMAL, NULL, 256);
            if (ret & 256)
            {
                delete database;
                database = CFileDB::Construct(fp);
                FileDB::DBs[i] = database;
                if (FileExists(fp))
                {
                    DebugLog("Reloading %s from disk...\n", fp.c_str());
                    database->Load();
                }
                else
                {
                    DebugLog("Tried reloading %s from disk, but file doesn't exist?\n", fp.c_str());
                }
            }

            manager->DoBreak();
        }

    }

    DoSectionHeader(manager, L"Debug");
    if (manager->DoInline(L"try reload resources", GTS_T5, STATE_NORMAL, NULL, 256) & 256)
        ReloadModifiedResources();
    manager->DoBreak();
    if (manager->DoInline(L"print loaded resources", GTS_T5, STATE_NORMAL, NULL, 256) & 256)
        PrintLoadedResources();
    manager->DoBreak();


    bool* pod_level = ((bool*)gGame) + 0x161;

    if (manager->DoInline(L"edit mode hack", GTS_T5, gGame->EditMode ? STATE_TOGGLE : STATE_NORMAL, NULL, 256) & 256)
        gGame->EditMode = !gGame->EditMode;

    if (manager->DoInline(L"pod mode hack", GTS_T5, *pod_level ? STATE_TOGGLE : STATE_NORMAL, NULL, 256) & 256)
        *pod_level = !*pod_level;
    
}

void DoConfigSubmenu(CGooeyNodeManager* manager)
{
    ConfigMap::iterator it;
    for (it = gConfigMap.begin(); it != gConfigMap.end(); ++it)
    {
        DoSectionHeader(manager, (wchar_t*)it->first);
        if (manager->StartFrame())
        {
            manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
            manager->AddFrameColumn(SizingBehaviour::Relative(0.5f), LM_JUSTIFY_START);
            manager->AddFrameColumn(SizingBehaviour::Relative(0.5f), LM_JUSTIFY_END);
            manager->SetFrameBorders(0.0f, 0.0f);
            manager->SetFrameDefaultChildSpacing(32.0f, 16.0f);

            for (CConfigOption* opt = it->second; opt != NULL; opt = opt->GetNext())
            {


                u32 input_mask = 256;
                if (opt->GetType() == OPT_FLOAT)
                {
                    input_mask |= 0x40;
                    input_mask |= 0x80;
                }

                u32 result = manager->DoInline(opt->GetDisplayName(), GTS_T5, STATE_NORMAL, NULL, input_mask);
                
                switch (opt->GetType())
                {
                    // DPAD
                    // 0x1
                    // 0x2
                    // 0x4
                    // 0x8

                    // STICK
                    // 0x10
                    // 0x20
                    // 0x40
                    // 0x80
                    
                    // 0x100 = CROSS
                    // 0x200 = CIRCLE


                    case OPT_BOOL:
                    {
                        CConfigBool& b = *(CConfigBool*)opt;
                        if (result & 256) b = !b;
                        manager->DoText(b ? (wchar_t*)L"true" : (wchar_t*)L"false", GTS_T5);

                        break;
                    }
                    case OPT_FLOAT:
                    {
                        wchar_t fstr[256];
                        CConfigFloat& f = *(CConfigFloat*)opt;
                        
                        if (result & 0x80) f.Increment(); // 0x10 = DPAD_UP
                                                          // 0x40 = DPAD_LEFT
                        if (result & 0x40) f.Decrement(); // 0x80 = DPAD_RIGHT

                        FormatString<256>(fstr, L"%.1f", (float)f);
                        manager->DoText(fstr, GTS_T5);
                        
                        break;
                    }
                    default:
                    {
                        manager->DoText(L"<N/A>", GTS_T5);
                        break;
                    }
                }
                manager->DoBreak();
            }

            manager->EndFrame();
        }

        manager->DoBreak();
    }
}

enum AlearPageType {
    SMP_CONFIG,
    SMP_TOOLS,
    SMP_SERVER,
    SMP_GAMEDATA,
    SMP_DEBUG,
    SMP_SERVERS,
    SMP_SYNC,
    SMP_SYNC_DEBUG
};

enum AlearSubPageType {
    SMSP_NONE,
    SMSP_UPLOAD_DATABASE,
    SMSP_VIRTUAL_FILESYSTEM,
    SMSP_CHANGE_HISTORY,
    SMSP_DOWNLOADS
};

void DoServersSubmenu(CGooeyNodeManager* manager)
{
    DoSectionHeader(manager, L"Instances");

    int server_index = gServerSwitcher->GetServerIndex();
    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
        manager->SetFrameBorders(0.0f, 0.0f);
        manager->SetFrameDefaultChildSpacing(32.0f, 16.0f);

        for (int i = 0; i < gServerSwitcher->GetNumServers(); ++i)
        {
            wchar_t* server_name = gServerSwitcher->GetServerName(i);
            u32 result = manager->DoInline(server_name, GTS_T5, server_index == i ? STATE_TOGGLE : STATE_NORMAL, NULL, 256);
            if (result & 256) gServerSwitcher->Switch(i);
            manager->DoBreak();
        }

        manager->EndFrame();
    }
}

bool IsValidDatabase(CFileDB* database)
{
    if (database == NULL) return false;
    if (database->Files.size() == 0) return false;

    const char* system_paths[] = 
    {
        "/gamedata/alear/boot.map",
        gSyncDatabasePath,
        "/output/brg_patch.map",
        "/output/blurayguids.map",
        "/output/brg_rnp.map"
    };

    for (int i = 0; i < ARRAY_LENGTH(system_paths); ++i)
    {
        if (strstr(database->Path.c_str(), system_paths[i]) != NULL)
            return false;
    }

    return true;
}

const char* GetPlural(int count)
{
    if (count > 1) return "files";
    return "file";
}

void DoSyncSubpageChangeHistory(CGooeyNodeManager* manager, bool first_open)
{
    DoSectionHeader(manager, L"Recent History");
    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);


        manager->AddFrameColumn(SizingBehaviour::Relative(0.77f), LM_JUSTIFY_START);
        manager->AddFrameColumn(SizingBehaviour::Relative(0.23f), LM_JUSTIFY_END);

        // manager->AddFrameColumn(SizingBehaviour::Relative(0.47f), LM_JUSTIFY_START);
        // manager->AddFrameColumn(SizingBehaviour::Relative(0.23f), LM_JUSTIFY_START);
        // manager->AddFrameColumn(SizingBehaviour::Relative(0.1f), LM_JUSTIFY_END);
        // manager->AddFrameColumn(SizingBehaviour::Relative(0.1f), LM_JUSTIFY_END);
        // manager->AddFrameColumn(SizingBehaviour::Relative(0.1f), LM_JUSTIFY_END);

        manager->SetFrameBorders(0.0f, 0.0f);
        manager->SetFrameDefaultChildSpacing(32.0f, 16.0f);

        // manager->DoText(L"Change", GTS_T3);
        // manager->DoText(L"Date", GTS_T3);

        // manager->DoText(L"Author", GTS_T3);
        // manager->DoText(L"Date", GTS_T3);
        // manager->DoText(L"+", GTS_T3);
        // manager->DoText(L"-", GTS_T3);
        // manager->DoText(L"/", GTS_T3);

        manager->DoBreak();

        for (int i = 0; i < MAX_COMMIT_ENTRIES; ++i)
        {
            SCommitData& commit = gCommitHistory[i];
            if (StringLength(commit.Author) == 0) continue;

            wchar_t message[256] = { 0 };
            wchar_t date[32] = { 0 };

            FormatString<32>(date, L"%s", commit.PublishDate);


            if (commit.Additions == 0 && commit.Changes == 0 && commit.Deletes == 0)
            {
                FormatString<256>(message, L"%s did literally nothing", commit.Author);
            }
            else if (commit.Additions != 0 && commit.Changes == 0)
            {
                FormatString<256>(message, L"%s added %d %s", commit.Author, commit.Additions, GetPlural(commit.Additions));
            }
            else if (commit.Additions == 0 && commit.Changes != 0)
            {
                FormatString<256>(message, L"%s changed %d %s", commit.Author, commit.Changes, GetPlural(commit.Changes));
            }
            else if (commit.Additions != 0 && commit.Changes != 0)
            {
                FormatString<256>(message, L"%s added %d %s and changed %d %s", commit.Author, commit.Additions, GetPlural(commit.Additions), commit.Changes, GetPlural(commit.Changes));
            }
            else if (commit.Deletes != 0)
            {
                FormatString<256>(message, L"%s deleted %d %s", commit.Author, commit.Deletes, GetPlural(commit.Deletes));
            }

            // manager->DoInline(message, GTS_T5, STATE_NORMAL, NULL, 256);
            manager->DoText(message, GTS_T5);
            manager->DoText(date, GTS_T5);

            manager->DoBreak();
        }

        manager->EndFrame();
    }

}

void DoDeleteButton(CGooeyNodeManager* manager, u64 uid, CBaseNode* file)
{
    if ((GetSyncServerPermissions() & PERMISSIONS_DELETE) != 0)
    {
        u32 res = manager->DoInline(uid, L"Delete", GTS_T5, file->IsInRecyclingBin() ? STATE_TOGGLE : STATE_NORMAL, NULL, 256);
        if (res & 256)
        {
            file->ToggleDelete();
            ForceStartMenuUpdate();
        }
    }
    else manager->DoSpacer();
}

void DoSyncSubpageFilesystem(CGooeyNodeManager* manager, bool first_open)
{
    static bool focus_next_frame = false;

    if ((GetSyncServerPermissions() & PERMISSIONS_READ) == 0)
    {
        ForceStartMenuUpdate();
        gPopAlearSubPage = true;
        return;
    }

    bool can_delete = (GetSyncServerPermissions() & PERMISSIONS_DELETE) != 0;
    if (first_open) ReloadSyncFilesystem();

    if (!gShowRecycleBin)
    {
        CRawVector<const wchar_t*> fragments;

        CFolderNode* node = gSelectedFolder;
        while (node->GetParent() != NULL)
        {
            fragments.push_back(node->GetName());
            node = (CFolderNode*)node->GetParent();
        }

        if (fragments.size() != 0)
        {
            MMString<wchar_t> title;
            for (int i = fragments.size() - 1; i >= 0; --i)
            {
                title += fragments[i];
                if (i - 1 >= 0) title += L"/";
            }

            DoSectionHeader(manager, title.c_str());
        }
        else DoSectionHeader(manager, L"Filesystem");
    } else DoSectionHeader(manager, L"Recycling Bin");

    if (!manager->StartFrameNamed(PRIMARY_FRAME_UID)) return;
    manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
    manager->SetFrameSizing(SizingBehaviour::Contents(), 400.0f);
    manager->SetFrameBorders(0.0f, 0.0f);

    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Relative(0.9f));
        // manager->AddFrameColumn(SizingBehaviour::Relative(0.8f), LM_JUSTIFY_START);
        // manager->AddFrameColumn(SizingBehaviour::Relative(0.2f), LM_JUSTIFY_END);
        manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
        manager->SetFrameBorders(0.0f, 0.0f);
        manager->SetFrameDefaultChildSpacing(32.0f, 16.0f);

        CFolderNode* queued_node = NULL;
        u64 item_uid = FIRST_SELECTION_UID;
        u64 action_uid = FIRST_SECONDARY_SELECTION_UID;

        CFolderNode* root = gShowRecycleBin ? &gRecyclingBin : gSelectedFolder;

        for (int i = 0; i < root->Folders.size(); ++i)
        {
            u64 uid = item_uid++;
            CFolderNode* folder = root->Folders[i];
            if (manager->DoInline(uid, folder->GetName(), GTS_T5, STATE_NORMAL, NULL, 256) & 256)
                queued_node = folder;

            // manager->DoSpacer();
            manager->DoBreak();
        }

        for (int i = 0; i < root->Files.size(); ++i)
        {
            CBaseNode* file = root->Files[i];
            u64 uid = item_uid++;

            // DoDeleteButton(manager, action_uid++, file);

            u32 input_mask = 256;
            if (can_delete) input_mask |= 0x800;

            if (manager->DoInline(item_uid, file->GetName(), GTS_T5, (file->IsInRecyclingBin() && !gShowRecycleBin) ? STATE_TOGGLE : STATE_NORMAL, NULL, input_mask) & 0x800)
            {
                file->ToggleDelete();
                ForceStartMenuUpdate();
            }

            manager->DoBreak();
        }

        if (focus_next_frame)
        {
            manager->EnsureNodeOrDescendantHasFocus(gSelectedFolder->LastSelectedUID);
            focus_next_frame = false;
        }

        if (manager->CurrentHighlightNode != NULL)
        {
            u64 uid = manager->CurrentHighlightNode->UID;
            if (uid > FIRST_SELECTION_UID && uid < FIRST_SECONDARY_SELECTION_UID)
                gSelectedFolder->LastSelectedUID = uid;
        }
        
        if (queued_node != NULL)
        {
            focus_next_frame = true;
            gSelectedFolder = queued_node;
            ForceStartMenuUpdate();
        }

        manager->EndFrame();
    }

    if (can_delete)
    {
        manager->DoHorizontalBreak(GBS_SOLID, v2(0.5f, 0.5f));

        if (manager->StartFrame())
        {
            manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Relative(0.1f));
            manager->SetFrameBorders(0.0f, 0.0f);
            manager->SetFrameLayoutMode(LM_JUSTIFY_END, LM_CENTERED);
            
            if (gShowRecycleBin)
            {
                if (manager->DoInline(L"Empty", GTS_T5, STATE_NORMAL, NULL, 256) & 256)
                {
                    CRawVector<CGUID> guids;
                    for (int i = 0; i < gRecyclingBin.Files.size(); ++i)
                    {
                        CFileNode* file = (CFileNode*)gRecyclingBin.Files[i];
                        guids.push_back(file->GetGUID());
                    }

                    DeleteResources(guids);
                }
            }

            if (manager->DoInline(0x52454359ull, L"Recycling Bin", GTS_T5, gShowRecycleBin ? STATE_TOGGLE : STATE_NORMAL, NULL, 256) & 256)
            {
                gShowRecycleBin = !gShowRecycleBin;
                ForceStartMenuUpdate();
            }

            manager->EndFrame();
        }
    }

    if (manager->EndFrame(0x200) & 0x200)
    {
        if (!gShowRecycleBin)
        {
            CFolderNode* parent = (CFolderNode*)gSelectedFolder->GetParent();
            if (parent != NULL) 
            {
                gSelectedFolder = parent;
                focus_next_frame = true;
            }
            else
            {
                gPopAlearSubPage = true;
            }
        } else gShowRecycleBin = false;

        ForceStartMenuUpdate();
    }

    if (first_open) manager->EnsureNodeOrDescendantHasFocus(FIRST_SELECTION_UID);
    manager->EnsureNodeOrDescendantHasFocus(PRIMARY_FRAME_UID);
}

void DoSyncSubpageDatabases(CGooeyNodeManager* manager, bool first_open)
{
    if ((GetSyncServerPermissions() & PERMISSIONS_UPLOAD) == 0)
    {
        ForceStartMenuUpdate();
        gPopAlearSubPage = true;
        return;
    }

    if (first_open)
    {
        ReloadPublishDatabases();
    }

    DoSectionHeader(manager, L"Select a database to upload");

    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
        manager->SetFrameBorders(0.0f, 0.0f);
        manager->SetFrameDefaultChildSpacing(32.0f, 16.0f);

        CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
        MMString<wchar_t> path;

        
        u64 selection_uid = FIRST_SELECTION_UID;
        for (CFilePath* it = gPublishDatabases.begin(); it != gPublishDatabases.end(); ++it)
        {
            CFilePath fp = (*it);
            GetRelativePath(path, fp);
            u32 result = manager->DoInline(selection_uid++, path.c_str(), GTS_T5, STATE_NORMAL, NULL, 256);
            if (result & 256)
            {
                CFileDB* database = CFileDB::Construct(fp);
                if (database != NULL)
                {
                    if (database->Load() == REFLECT_OK) UploadDatabase(database);
                    else delete database; // Upload thread func will delete the database
                }
            }

            manager->DoBreak();
        }

        for (CFileDB** it = FileDB::DBs.begin(); it != FileDB::DBs.end(); ++it)
        {
            CFileDB* database = (*it);
            if (!IsValidDatabase(database)) continue;
            
            GetRelativePath(path, database->Path);
            u32 result = manager->DoInline(selection_uid++, path.c_str(), GTS_T5, STATE_NORMAL, NULL, 256);
            if (result & 256)
                UploadDatabase(database);

            manager->DoBreak();
        }

        manager->EndFrame();
    }

    if (first_open)
        manager->EnsureNodeOrDescendantHasFocus(FIRST_SELECTION_UID);
}

int DoSyncSubpage(CGooeyNodeManager* manager, AlearSubPageType subpage, bool first_open)
{
    switch (subpage)
    {
        case SMSP_UPLOAD_DATABASE:
        {
            DoSyncSubpageDatabases(manager, first_open);
            break;
        }

        case SMSP_CHANGE_HISTORY:
        {
            DoSyncSubpageChangeHistory(manager, first_open);
            break;
        }

        case SMSP_VIRTUAL_FILESYSTEM:
        {
            DoSyncSubpageFilesystem(manager, first_open);
            break;
        }

        default:
        {
            break;
        }
    }

    int flags = 0;
    
    if (gPopAlearSubPage)
    {
        flags |= 2;
        gPopAlearSubPage = false;
    }

    return flags;
}

void ReloadDatabase(CFilePath& fp)
{
    FileDB::Mutex.Enter(__FILE__, __LINE__);
    CFileDB** iter = FileDB::DBs.begin();
    for (; iter != FileDB::DBs.end(); iter++)
    {
        CFileDB* database = *iter;
        if (database->Path != fp) continue;

        delete database;
        database = CFileDB::Construct(fp);
        *iter = database;

        if (FileExists(fp))
        {
            DebugLog("Reloading %s from disk...\n", fp.c_str());
            database->Load();
        }
        else
        {
            DebugLog("Tried reloading %s from disk, but file doesn't exist?\n", fp.c_str());
        }

        break;
    }
    
    FileDB::Mutex.Leave();
}

void ReloadPendingDatabases()
{
    CCSLock _the_lock(&gReloadCS, __FILE__, __LINE__);

    CFilePath* iter = gReloadMap.begin();
    for (; iter != gReloadMap.end(); iter++)
        ReloadDatabase(*iter);
    
    if (gReloadMap.size() != 0)
    {
        gReloadMap.clear();
        ReloadReadonlyCaches();
        ReloadModifiedResources();
    }
}

void OnDatabaseFileChanged(CFilePath& fp)
{
    DebugLog("File has been updated on local filesystem! (%s)\n", fp.c_str());
    CCSLock _the_lock(&gReloadCS, __FILE__, __LINE__);
    gReloadMap.push_back(fp);
}

extern void OnStopTweaking(CThing* thing);
namespace AlearOptNativeFunctions
{
    void RefreshUsedItems(CScriptObjectPoppet* so_poppet)
    {
        GatherUsedPlanDescriptors();
    }

    void ReloadInventoryItem(CScriptObjectPoppet* so_poppet, u32 uid)
    {
        if (so_poppet == NULL) return;
        CPoppet* poppet = so_poppet->GetNativeObject();
        if (poppet == NULL) return;

        const CP<RLocalProfile>& prf = poppet->GetLocalProfile();
        if (!prf) return;

        CInventoryItem* item = prf->GetInventoryItemByUID(uid);
        if (item == NULL) return;

        CP<RPlan> plan = LoadResource<RPlan>(item->Plan, 0, STREAM_PRIORITY_DEFAULT, false);
        plan->BlockUntilLoaded();
        if (!plan->IsLoaded()) return;

        u32 category = plan->InventoryData.Category;
        if (category == 0 && (plan->InventoryData.Type & E_TYPE_COSTUME_MATERIAL) != 0)
            category = 2988363256ull;

        item->Details.Icon = plan->InventoryData.Icon;
        item->Details.NameTranslationTag = plan->InventoryData.NameTranslationTag;
        item->Details.DescTranslationTag = plan->InventoryData.DescTranslationTag;
        item->Details.LocationIndex = prf->AddString(plan->InventoryData.Location);
        item->Details.CategoryIndex = prf->AddString(category);
        item->Details.UserCreatedName = plan->InventoryData.UserCreatedName;
        item->Details.UserCreatedDescription = plan->InventoryData.UserCreatedDescription;
        item->Details.Type = plan->InventoryData.Type;
        item->Details.SubType = plan->InventoryData.SubType;
        item->Details.ToolType = plan->InventoryData.ToolType;
        item->Details.Creator = plan->InventoryData.Creator;
        item->Details.HighlightSound = plan->InventoryData.HighlightSound;
        item->Details.LevelUnlockSlotID = plan->InventoryData.LevelUnlockSlotID;
        
        prf->SetViewsDirtyIfTheyContainItem(uid);
    }

    void DoGamedataSubmenu(CScriptObjectGooey* gooey)
    {
        if (gooey == NULL) return;
        CScriptedGooeyWrapper* wrapper = gooey->GetNativeObject();
        if (wrapper == NULL) return;
        DoGamedataSubmenu(wrapper->Manager);
    }

    int DoSyncSubpage(CScriptObjectGooey* gooey, AlearSubPageType subpage, bool first_open)
    {
        if (gooey == NULL) return 0;
        CScriptedGooeyWrapper* wrapper = gooey->GetNativeObject();
        if (wrapper == NULL) return 0;
        return DoSyncSubpage(wrapper->Manager, subpage, first_open);
    }

    void DoConfigSubmenu(CScriptObjectGooey* gooey)
    {
        if (gooey == NULL) return;
        CScriptedGooeyWrapper* wrapper = gooey->GetNativeObject();
        if (wrapper == NULL) return;
        DoConfigSubmenu(wrapper->Manager);
    }

    void DoServersSubmenu(CScriptObjectGooey* gooey)
    {
        if (gooey == NULL) return;
        CScriptedGooeyWrapper* wrapper = gooey->GetNativeObject();
        if (wrapper == NULL) return;
        DoServersSubmenu(wrapper->Manager);
    }

    void Register()
    {
        RegisterNativeFunction("Start_Menu", "DoGamedataSubmenu__Q5Gooey", true, NVirtualMachine::CNativeFunction1V<CScriptObjectGooey*>::Call<DoGamedataSubmenu>);
        RegisterNativeFunction("Start_Menu", "DoConfigSubmenu__Q5Gooey", true, NVirtualMachine::CNativeFunction1V<CScriptObjectGooey*>::Call<DoConfigSubmenu>);
        RegisterNativeFunction("Start_Menu", "DoServersSubmenu__Q5Gooey", true, NVirtualMachine::CNativeFunction1V<CScriptObjectGooey*>::Call<DoServersSubmenu>);
        RegisterNativeFunction("Start_Menu", "DoSyncSubpage__Q5Gooeyib", true, NVirtualMachine::CNativeFunction3<int, CScriptObjectGooey*, AlearSubPageType, bool>::Call<DoSyncSubpage>);
        
        RegisterNativeFunction("Alear", "IsSyncServerConnected__", true, NVirtualMachine::CNativeFunction0<bool>::Call<IsSyncServerConnected>);
        RegisterNativeFunction("Alear", "IsSyncServerConnecting__", true, NVirtualMachine::CNativeFunction0<bool>::Call<IsSyncServerConnecting>);
        RegisterNativeFunction("Alear", "TryConnectSyncServer__", true, NVirtualMachine::CNativeFunction0V::Call<TryConnectSyncServer>);
        RegisterNativeFunction("Alear", "GetSyncServerPermissions__", true, NVirtualMachine::CNativeFunction0<ESyncPermissions>::Call<GetSyncServerPermissions>);

        RegisterNativeFunction("Poppet", "ReloadFromInventory__i", false, NVirtualMachine::CNativeFunction2V<CScriptObjectPoppet*, u32>::Call<ReloadInventoryItem>);
        RegisterNativeFunction("Poppet", "RefreshUsedItems__", false, NVirtualMachine::CNativeFunction1V<CScriptObjectPoppet*>::Call<RefreshUsedItems>);
        RegisterNativeFunction("Poppet", "InvalidateTweakThing__Q5Thing", true, NVirtualMachine::CNativeFunction1V<CThing*>::Call<OnStopTweaking>);
    }
}

#include <Hash.h>
#include "TweakShape.h"
extern "C" void _alear_levelupdate_hook();
void InitAlearOptUiHooks()
{
    AlearOptNativeFunctions::Register();
    TweakShapeNativeFunctions::Register();
    MH_Poke32(0x00015874, B(&_alear_levelupdate_hook, 0x00015874));
}