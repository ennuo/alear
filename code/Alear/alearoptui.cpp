#include "alearoptui.h"
#include "fwatch.h"

#include <hook.h>
#include <mmalex.h>
#include <ppcasm.h>
#include <printf.h>
#include <filepath.h>
#include <refcount.h>
#include <GuidHashMap.h>
#include <mem_stl_buckets.h>

#include <cell/DebugLog.h>

#include <gooey/GooeyNodeManager.h>
#include <gooey/GooeyScriptInterface.h>

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

#include <thing.h>
#include <PartPhysicsWorld.h>
#include <FartRO.h>
#include <Resource.h>
#include <ResourceLevel.h>
#include <ResourceSystem.h>
#include <ResourceScript.h>
#include <CritSec.h>
#include <ResourceGFXTexture.h>


#ifdef __SM64__
extern void UpdateMarioAvatars();
extern void ClearMarioAvatars();
#endif

CVector<CFilePath> gReloadMap;
CCriticalSec gReloadCS("ReloadCS");

void ReloadReadonlyCaches()
{
    CCSLock _res_lock(&gResourceCS, __FILE__, __LINE__);
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
    CVector<RelinkTarget> relinker;
    CVector<CResource*> scripts;

    bool relink_scripts = false;

    {
        CCSLock _the_lock(&gResourceCS, __FILE__, __LINE__);
        for (int i = 0; i < gResourceArray.size(); ++i)
        {
            CResource* resource = gResourceArray[i];
            EResourceType type = resource->GetResourceType();
            CGUID guid = resource->GetGUID();
            if (guid == 0) continue;

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
    CCSLock _the_lock(&gResourceCS, __FILE__, __LINE__);
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
        DebugLog("\tg0x%08x (h%s) -> %s\n", row->FileGuid.guid, hash, row->FilePathX);
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
            manager->DoInline(wstr.c_str(), GTS_T3, STATE_NORMAL, NULL, 0);
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
            u32 ret = manager->DoInline(wstr.c_str(), GTS_T3, STATE_NORMAL, NULL, 256);
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
    if (manager->DoInline(L"try reload resources", GTS_T3, STATE_NORMAL, NULL, 256) & 256)
        ReloadModifiedResources();
    manager->DoBreak();
    if (manager->DoInline(L"print loaded resources", GTS_T3, STATE_NORMAL, NULL, 256) & 256)
        PrintLoadedResources();
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
        //ReloadReadonlyCaches();
        ReloadModifiedResources();
    }
}

void OnDatabaseFileChanged(CFilePath& fp)
{
    DebugLog("File has been updated on local filesystem! (%s)\n", fp.c_str());
    CCSLock _the_lock(&gReloadCS, __FILE__, __LINE__);
    gReloadMap.push_back(fp);
}

void OnReleaseLevel()
{
    #ifdef __SM64__
    ClearMarioAvatars();
    #endif
}

void OnUpdateLevel()
{
    #ifdef __SM64__
    UpdateMarioAvatars();
    #endif
    
    ReloadPendingDatabases();
}

namespace AlearOptNativeFunctions
{
    void DoGamedataSubmenu(CScriptObjectGooey* gooey)
    {
        if (gooey == NULL) return;
        CScriptedGooeyWrapper* wrapper = gooey->GetNativeObject();
        if (wrapper == NULL) return;
        DoGamedataSubmenu(wrapper->Manager);
    }

    void Register()
    {
        RegisterNativeFunction("Start_Menu", "DoGamedataSubmenu__Q5Gooey", true, NVirtualMachine::CNativeFunction1V<CScriptObjectGooey*>::Call<DoGamedataSubmenu>);
    }
}

extern "C" void _alear_levelupdate_hook();
void InitAlearOptUiHooks()
{
    AlearOptNativeFunctions::Register();
    MH_Poke32(0x00015874, B(&_alear_levelupdate_hook, 0x00015874));
}