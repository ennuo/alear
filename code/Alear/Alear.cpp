#include "Alear.h"
#include "json.h"
#include "ServerSwitcher.h"
#include "resources/ResourcePins.h"
#include "resources/ResourceHooks.h"
#include "customization/Styles.h"
#include "AlearVM.h"
#include "Portal.h"
#include "Powerup.h"
#include "AlearConfig.h"
#include "customization/PodStyles.h"
#include "AlearDebugCamera.h"
#include "AlearStartMenu.h"
#include "AlearShared.h"
#include "FileWatcher.h"
#include "PinSystem.h"
#include "OutfitSystem.h"
#include "RenderJoint.h"
#include "Sync/Bootstrap.h"
#include <LooksMenu.h>

#ifdef __SM64__
#include <sm64/init.h>
#endif 

#include "MMString.h"
#include "cell/thread.h"
#include "LoadingScreen.h"
#include <Serialise.h>
#include <Explode.h>
#include <Launcher.h>

#include "RPCS3.h"
#include "ppcasm.h"
#include "PoppetOutlineShapes.h"

#include "ResourceSystem.h"
#include <FartRO.h>
#include <System.h>
#include <resources/ResourceAnimatedTexture.h>
#include <AlearHooks.h>

extern "C" void _gfxbind_hook_naked();
extern void InitGooeyNetworkHooks();
extern bool InitTweakSettings();
extern void LoadSackboyPolygon();
extern void LoadBallPolygon();
extern bool InitializeExplosiveStyles();
extern void AttachWebternateHooks();
extern void AttachFloatyFluidHooks();

bool AlearCheckPatch();

bool AlearEpilogue()
{
    DebugLog("FileDB::DBs:\n");
    {
        CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
        for (int i = 0; i < FileDB::DBs.size(); ++i)
        {
            gFileWatcher->AddFile(FileDB::DBs[i]->Path, OnDatabaseFileChanged);
            DebugLog("\t[0x%x]: %s\n", (u32)FileDB::DBs[i], FileDB::DBs[i]->Path.c_str());
        }
    }


    {
        CCSLock _depot_mutex(&sync::DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < sync::Depots.size(); ++i)
        {
            const sync::depot& d = sync::Depots[i];
            if (d.IsLocal())
                gFileWatcher->AddFile(d.Database->Path, OnDatabaseFileChanged);
        }
    }

    CFartManyRO* fart = (CFartManyRO*)gCaches[CT_READONLY];
    {
        CCSLock _cache_lock(&fart->Mutex, __FILE__, __LINE__);
        for (u32 i = 0; i < fart->Farts.size(); ++i)
            gFileWatcher->AddFile(fart->Farts[i]->fp, OnCacheFileChanged);
    }


    LoadCursorSprites();
    LoadRecordingShaders();
    LoadPostProcessingShaders();
    LoadSackboyPolygon();
    LoadBallPolygon();
    InitializeExplosiveStyles();

    alear::LoadConfig();
    
    return true;
}

// need this to load before a specific init step, so going to separate it out
CInitStep gEmoteInitStep = { "Emotes", NULL, LoadEmotes, UnloadEmotes, NULL, false, NULL };
CInitStep gSyncStep = { "Ikaros", NULL, sync::Open, sync::Close, NULL, false, NULL };
CInitStep gAlearInitSteps[] =
{
    { "Alear Patch Validator", NULL, AlearCheckPatch, NULL, false, NULL },
    { "Alear Server Switcher", NULL, AlearInitServerSwitcher, NULL, NULL, false, NULL },
    #ifdef __CINEMACHINE__
    { "Alear Cinemachine", NULL, LoadCameraClips, NULL, NULL, false, NULL },
    #endif
    { "Alear File Watcher", NULL, InitFileWatcher, CloseFileWatcher, NULL, false, NULL },
    { "Slap Styles", NULL, LoadSlapStyles, UnloadSlapStyles, NULL, false, NULL },
    { "Outfit Lists", NULL, LoadOutfits, NULL, NULL, false, NULL },
    { "Joint Meshes", NULL, LoadJointMeshes, NULL, NULL, false, NULL },
    { "Alear Gooey Network Extensions", NULL, InitTweakSettings, false, NULL },
    { "Alear Epilogue", NULL, AlearEpilogue, NULL, NULL, false, NULL },
    #ifdef __SM64__
    { "Super Mario 64", NULL, NULL, CloseMarioLib, InitMarioLib, false, NULL },
    #endif
    // Need a NULL entry to indicate the end of the initialization steps
    { NULL, NULL, NULL, NULL, NULL, false, NULL }
};

#include <Directory.h>
void AlearSetupDatabase()
{
    CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
    sync::Database.Path = CFilePath(FPR_ALEAR, "sync/cache.map");

    CVector<MMString<char> > paths;
    if (!FileLoad(CFilePath(FPR_ALEAR, "config/databases.txt"), paths))
    {
        paths.push_back("output/brg_patch.map");
        paths.push_back("output/blurayguids.map");
    }

    for (u32 i = 0; i < paths.size(); ++i)
    {
        const MMString<char>& path = paths[i];
        CFilePath fp;

        if (path.compare("output/blurayguids.map") == 0)
            fp = CFilePath(FPR_BLURAY, "output/blurayguids.map");
        else if (gGameDataReady)
            fp = CFilePath(FPR_GAMEDATA, path.c_str());
        else
        {
            MMLog("Skipping load of database %s as gamedata isn't ready\n", path.c_str());
            continue;
        }

        if (fp == sync::Database.Path)
        {
            FileDB::DBs.push_back(&sync::Database);
            continue;
        }

        if (!FileExists(fp))
        {
            MMLog("Skipping load of database %s as file does not exist locally\n", path.c_str());
            continue;
        }

        FileDB::DBs.push_back(new CFileDB(fp));
    }

    for (int i = 0; i < FileDB::DBs.size(); ++i)
        FileDB::DBs[i]->Load();
}

#include <gooey/GooeyImage.h>
#include <Translate.h>

extern MAKE_THREAD_FUNCTION(MainLoadingThread);
extern MAKE_THREAD_FUNCTION(MainSlowThread);
void AlearStartup()
{
    DebugLog("Alear version v%f build date: " __DATE__ " time: " __TIME__ "\n", mmalex::sqrtf(ALEAR_VERSION));

    DebugLog("Injecting init steps into startup...\n");
    DebugLog("First CInitStep: %s\n", gInitSteps[0].DebugText);
    DebugLog("First ps3test1 CInitStep: %s\n", gPs3Test1InitSteps[0].DebugText);
    DebugLog("First Alear CInitStep: %s\n", gAlearInitSteps[0].DebugText);
    DebugLog("SPRX TOC Base: %x\n", gTocBase);

    DebugLog("sizeof(CGooeyImage) = 0x%08x\n", sizeof(CGooeyImage));
    MMLog("MakeLamsKeyID(%s) = %08x", "GRAVITYRUSH_COSTUME_KAT_COSTUME_NAME", MakeLamsKeyID("GRAVITYRUSH_COSTUME_KAT_COSTUME_NAME"));

    // Setup all our hooks
    InitSharedHooks();
    InitLogicSystemHooks();
    AttachExplosionHooks();
    MH_InitHook((void*)0x0057d548, (void*)&AlearSetupDatabase);
    MH_Poke32(0x000997c8, B(&_gfxbind_hook_naked, 0x000997c8));
    AlearInitVMHook();
    AlearInitPortalHook();
    InitStyleHooks();
    InitResourceHooks();
    InitGooeyNetworkHooks();
    AlearInitCreatureHook();
    AlearInitConf();
    InitPodStyles();
    InitCameraHooks();
    InitPinHooks();
    InitOutfitHooks();
    InitAlearOptUiHooks();
    AttachWebternateHooks();
    AttachFloatyFluidHooks();
    AttachLooksMenuHooks();
    Launcher::Attach();
    
    MH_PokeHook(0x0040b678, SetJetpackTether);
    MH_PokeHook(0x0040a9f4, CollectGun);
    MH_PokeHook(0x0040a92c, SetScubaGear);

    MH_PokeHook(0x00090538, MainLoadingThread);
    MH_PokeHook(0x00090060, MainSlowThread);
    MH_PokeHook(0x0057d008, FileDB::RemapLocalGUID);
    MH_PokeHook(0x0057ca30, FileDB::Destroy);
    
    MH_Poke32(0x0001da24, 0x39200001);
    MH_PokeBranch(0x000ebc6c, &_on_post_sackboy_animation_update_hook);
    

    // This module gets initialized by replacing the function that normally
    // adds the init steps for ps3test1, so make sure we're adding them after our own.
    AddInitSteps(gPs3Test1InitSteps);
    AddInitSteps(gAlearInitSteps);

    CInitStep* target = gPs3Test1InitSteps;
    while (strcmp(target->DebugText, "SceneGraph") != 0) target++;
    target->ChainTo = &gEmoteInitStep;
    gEmoteInitStep.ChainTo = target + 1;

    target = gInitSteps;
    while (strcmp(target->DebugText, "NetworkUtilities") != 0) target++;
    target->ChainTo = &gSyncStep;
    gSyncStep.ChainTo = target + 1;

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

#include <gooey/GooeyRender.h>

bool AlearCheckPatch()
{
    if (!IsUsingLLVM()) return true;
    GeneratePatchYML();

    CTextState state(0xd, 0, 0, 0x0, 0, NULL);

    MultiByteToTChar(state.title, "Alear", NULL);
    MultiByteToTChar(state.text, "Alear requires that you run RPCS3 with the PPU Decoder setting set to 'Interpreter' mode.", NULL);

    NHUD::PreloadGlyphs(gFont[FONT_OMNES_BLACK], state.title.c_str(), NULL);
    NHUD::PreloadGlyphs(gFont[FONT_HELVETICA], state.text.c_str(), NULL);

    void* handle = SetActiveLoadingScreen(&state, NULL, true);
    ThreadSleep(10000);
    CancelActiveLoadingScreen(handle, false, 0);

    

    return false;
}