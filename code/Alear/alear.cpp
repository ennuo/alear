#include "alear.h"
#include "json.h"
#include "serverswitcher.h"
#include "styles.h"
#include "alearvm.h"
#include "portal.h"
#include "powerup.h"
#include "alearconf.h"
#include "podstyles.h"

#include "MMString.h"
#include "cell/thread.h"
#include "LoadingScreen.h"
#include "rpcs3.h"
#include "ppcasm.h"

#include "ResourceSystem.h"


extern "C" void _gfxbind_hook_naked();

bool AlearCheckPatch();
bool AlearEpilogue()
{
    DebugLog("Performing init steps...\n");

    *((CP<RTranslationTable>*)&gAlearTrans) = LoadResourceByKey<RTranslationTable>(3709465117, 0, STREAM_PRIORITY_DEFAULT);

    DebugLog("FileDB::DBs:\n");
    CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
    for (int i = 0; i < FileDB::DBs.size(); ++i)
    {
        DebugLog("\t[0x%x]: %s\n", (u32)FileDB::DBs[i], FileDB::DBs[i]->Path.c_str());
    }
    
    return true;
}

CInitStep g_AlearInitSteps[] =
{
    { "Alear Patch Validator", NULL, AlearCheckPatch, NULL, false, NULL },
    { "Alear Epilogue", NULL, AlearEpilogue, NULL, NULL, false, NULL },
    { "Alear Server Switcher", NULL, AlearInitServerSwitcher, NULL, NULL, false, NULL },
    // Need a NULL entry to indicate the end of the initialization steps
    { NULL, NULL, NULL, NULL, NULL, false, NULL }
};

/// @brief Loads databases from a user-provided config file
/// @return Whether the operation was successful 
bool AlearLoadDatabaseConfiguration()
{
    CFilePath fp(FPR_GAMEDATA, "/gamedata/alear/databases.json");
    if (!FileExists(fp))
    {
        DebugLog("No database configuration file exists at %s!\n", fp.c_str());
        // Not technically a fail, just doesn't exist
        return true;
    }

    DebugLog("Loading database config from %s\n", fp.c_str());
    char* json = FileLoadText(fp);
    if (json == NULL)
    {
        DebugLog("Failed to read database configuration file at %s!\n", fp.c_str());
        return false;
    }

    const int MAX_DATABASES = 16;
    const int MAX_FIELDS = MAX_DATABASES + 1;

    json_t pool[MAX_FIELDS];
    json_t const* root = json_create(json, pool, MAX_FIELDS);
    if (root == NULL)
    {
        DebugLog("Failed to parse database configuration!\n");
        return false;
    }

    if (json_getType(root) != JSON_ARRAY)
    {
        DebugLog("Root of database configuration file must be a JSON string array!\n");
        return false;
    }

    for (json_t const* element = json_getChild(root); element != NULL; element = json_getSibling(element))
    {
        if (json_getType(element) != JSON_TEXT)
        {
            DebugLog("Element in JSON array is invalid! (Not a string)\n");
            continue;
        }

        const char* path = json_getValue(element);
        if (path == NULL)
        {
            DebugLog("Database element filepath was NULL!\n");
            continue;
        }

        CFilePath database_fp(FPR_GAMEDATA, (char*)path);
        if (!FileExists(database_fp))
        {
            DebugLog("Database at %s doesn't exist, skipping...\n", database_fp.c_str());
            continue;
        }

        DebugLog("Adding database %s...\n", database_fp.c_str());
        FileDB::DBs.push_back(CFileDB::Construct(database_fp));
    }

    return true;
}

void AlearSetupDatabase()
{
    CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);

    CFilePath alear_fp(FPR_GAMEDATA, "/gamedata/alear/boot.map");
    if (FileExists(alear_fp))
    {
        FileDB::DBs.push_back(CFileDB::Construct(alear_fp));
    }
    else DebugLog("WARNING: %s isn't present, this may cause issues!\n", alear_fp.c_str());

    AlearLoadDatabaseConfiguration();

    if (g_GameDataReady)
    {
        CFilePath patch_fp(FPR_GAMEDATA, "/output/brg_patch.map");
        if (FileExists(patch_fp))
            FileDB::DBs.push_back(CFileDB::Construct(patch_fp));
    }

    CFilePath fp(FPR_BLURAY, "/output/blurayguids.map");
    FileDB::DBs.push_back(CFileDB::Construct(fp));

    for (int i = 0; i < FileDB::DBs.size(); ++i)
        FileDB::DBs[i]->Load();
}

void AlearStartup()
{
    DebugLog("Alear version %d.%d build date: " __DATE__ " time: " __TIME__ "\n", ALEAR_MAJOR_VERSION, ALEAR_MINOR_VERSION);

    DebugLog("Injecting init steps into startup...\n");
    DebugLog("First CInitStep: %s\n", g_InitSteps[0].DebugText);
    DebugLog("First ps3test1 CInitStep: %s\n", g_Ps3Test1InitSteps[0].DebugText);
    DebugLog("First Alear CInitStep: %s\n", g_AlearInitSteps[0].DebugText);
    DebugLog("SPRX TOC Base: %x\n", g_TocBase);

    // Setup all our hooks
    MH_InitHook((void*)0x0057d548, (void*)&AlearSetupDatabase);
    MH_Poke32(0x000997c8, B(&_gfxbind_hook_naked, 0x000997c8));
    AlearInitVMHook();
    AlearInitPortalHook();
    AlearInitStyles();
    AlearInitCreatureHook();
    AlearInitConf();
    InitPodStyles();
    ServerSwitcherNativeFunctions::Register();
    

    // This module gets initialized by replacing the function that normally
    // adds the init steps for ps3test1, so make sure we're adding them after our own.
    AddInitSteps(g_Ps3Test1InitSteps);
    AddInitSteps(g_AlearInitSteps);

    DebugLog("g_InitSteps:\n");
    CInitStep* step = g_InitSteps, *last = NULL;
    while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
    {
        DebugLog("\t%s\n", step->DebugText);
        step = ((last = step)->ChainTo == NULL) ? ++step : last->ChainTo;
    }

    DebugLog("Module has been initialized!\n");
}

void AlearShutdown()
{
    
}

bool AlearCheckPatch()
{
    if (!IsUsingLLVM()) return true;

    CTextState state(0xd, 0, 0, 0x0, 0, NULL);

    MultiByteToTChar(state.title, "Alear", NULL);
    MultiByteToTChar(state.text, "Alear requires that you run RPCS3 with the PPU Decoder setting set to 'Interpreter' mode.", NULL);

    void* handle = SetActiveLoadingScreen(&state, NULL, true);
    ThreadSleep(10000);
    CancelActiveLoadingScreen(handle, false, 0);

    return false;
}