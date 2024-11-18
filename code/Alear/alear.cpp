#include "alear.h"
#include "json.h"
#include "serverswitcher.h"
#include "resources/ResourcePins.h"
#include "resources/ResourceHooks.h"
#include "customization/styles.h"
#include "alearvm.h"
#include "portal.h"
#include "powerup.h"
#include "alearconf.h"
#include "podstyles.h"
#include "alearcam.h"
#include "alearoptui.h"
#include "alearshared.h"
#include "fwatch.h"
#include "pins.h"

#ifdef __SM64__
#include <sm64/init.h>
#endif 

#include "MMString.h"
#include "cell/thread.h"
#include "LoadingScreen.h"
#include <Serialise.h>

#include "rpcs3.h"
#include "ppcasm.h"

#include "ResourceSystem.h"


extern "C" void _gfxbind_hook_naked();

bool AlearCheckPatch();
bool AlearEpilogue()
{
    DebugLog("FileDB::DBs:\n");
    CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
    for (int i = 0; i < FileDB::DBs.size(); ++i)
    {
        gFileWatcher->AddFile(FileDB::DBs[i]->Path, OnDatabaseFileChanged);
        DebugLog("\t[0x%x]: %s\n", (u32)FileDB::DBs[i], FileDB::DBs[i]->Path.c_str());
    }
    
    return true;
}

// need this to load before a specific init step, so going to separate it out
CInitStep gEmoteInitStep = { "Emotes", NULL, LoadEmotes, UnloadEmotes, NULL, false, NULL };
CInitStep gAlearInitSteps[] =
{
    { "Alear Patch Validator", NULL, AlearCheckPatch, NULL, false, NULL },
    { "Alear Server Switcher", NULL, AlearInitServerSwitcher, NULL, NULL, false, NULL },
    #ifdef __CINEMACHINE__
    { "Alear Cinemachine", NULL, LoadCameraClips, NULL, NULL, false, NULL },
    #endif
    { "Alear File Watcher", NULL, InitFileWatcher, CloseFileWatcher, NULL, false, NULL },
    { "Slap Styles", NULL, LoadSlapStyles, UnloadSlapStyles, NULL, false, NULL },
    { "Alear Epilogue", NULL, AlearEpilogue, NULL, NULL, false, NULL },
    #ifdef __SM64__
    { "Super Mario 64", NULL, NULL, CloseMarioLib, InitMarioLib, false, NULL },
    #endif
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

bool gEnableFHD = false;

void AlearHookHD()
{
    const int width = 1920;
    const int height = 1080;

    MH_Poke32(0x001df23c + 0x00, LI(4, width));
    MH_Poke32(0x001df23c + 0x04, LI(5, height));

    MH_Poke32(0x001df124 + 0x00, LI(4, width));
    MH_Poke32(0x001df124 + 0x04, LI(5, height));

    MH_Poke32(0x001df140 + 0x00, LI(4, width));
    MH_Poke32(0x001df140 + 0x04, LI(5, height));

    MH_Poke32(0x001df15c + 0x00, LI(4, width));
    MH_Poke32(0x001df15c + 0x04, LI(5, height));

    MH_Poke32(0x001df178 + 0x00, LI(4, width));
    MH_Poke32(0x001df178 + 0x04, LI(5, height));

    MH_Poke32(0x001df05c + 0x00, LI(4, width));
    MH_Poke32(0x001df05c + 0x04, LI(5, height));

    MH_Poke32(0x001df03c + 0x00, LI(4, width));
    MH_Poke32(0x001df03c + 0x04, LI(5, height));

    // PRT_L_BUFFER_720P2X
    MH_Poke32(0x001df07c + 0x00, LI(4, (width * 2)));
    MH_Poke32(0x001df07c + 0x04, LI(5, height));

    // Fix the scissor dimensions in the SetNiceState function
    MH_Poke32(0x003e28d8 + 0x00, LI(6, width));
    MH_Poke32(0x003e28d8 + 0x04, LI(7, height));
    // Change dimension when transferring image in Swap
    MH_Poke32(0x003e3f0c + 0x00, LI(3, width));
    MH_Poke32(0x003e3f0c + 0x04, LI(4, height));
}

void AlearSetupDatabase()
{
    CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);

    if (gEnableFHD)
    {
        CFilePath fp(FPR_GAMEDATA, "/gamedata/alear/hd/patch.map");
        if (FileExists(fp))
        {
            FileDB::DBs.push_back(CFileDB::Construct(fp));
        }
    }
    
    // CFilePath alear_fp(FPR_GAMEDATA, "/gamedata/alear/boot.map");
    // if (FileExists(alear_fp))
    // {
    //     FileDB::DBs.push_back(CFileDB::Construct(alear_fp));
    // }
    // else DebugLog("WARNING: %s isn't present, this may cause issues!\n", alear_fp.c_str());

    AlearLoadDatabaseConfiguration();

    if (gGameDataReady)
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
    DebugLog("First CInitStep: %s\n", gInitSteps[0].DebugText);
    DebugLog("First ps3test1 CInitStep: %s\n", gPs3Test1InitSteps[0].DebugText);
    DebugLog("First Alear CInitStep: %s\n", gAlearInitSteps[0].DebugText);
    DebugLog("SPRX TOC Base: %x\n", gTocBase);

    // Setup all our hooks
    InitSharedHooks();
    MH_InitHook((void*)0x0057d548, (void*)&AlearSetupDatabase);
    MH_Poke32(0x000997c8, B(&_gfxbind_hook_naked, 0x000997c8));
    AlearInitVMHook();
    AlearInitPortalHook();
    InitStyleHooks();
    InitResourceHooks();
    AlearInitCreatureHook();
    AlearInitConf();
    InitPodStyles();
    InitCameraHooks();
    InitPinHooks();
    InitAlearOptUiHooks();
    if (gEnableFHD) AlearHookHD();

    MH_Poke32(0x0001da24, 0x39200001);
    

    // This module gets initialized by replacing the function that normally
    // adds the init steps for ps3test1, so make sure we're adding them after our own.
    AddInitSteps(gPs3Test1InitSteps);
    AddInitSteps(gAlearInitSteps);

    CInitStep* target = gPs3Test1InitSteps;
    while (strcmp(target->DebugText, "SceneGraph") != 0) target++;
    target->ChainTo = &gEmoteInitStep;
    gEmoteInitStep.ChainTo = target + 1;

    DebugLog("gInitSteps:\n");
    CInitStep* step = gInitSteps, *last = NULL;
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
    GeneratePatchYML();

    CTextState state(0xd, 0, 0, 0x0, 0, NULL);

    MultiByteToTChar(state.title, "Alear", NULL);
    MultiByteToTChar(state.text, "Alear requires that you run RPCS3 with the PPU Decoder setting set to 'Interpreter' mode.", NULL);

    void* handle = SetActiveLoadingScreen(&state, NULL, true);
    ThreadSleep(10000);
    CancelActiveLoadingScreen(handle, false, 0);

    

    return false;
}