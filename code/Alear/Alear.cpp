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

#ifdef __SM64__
#include <sm64/init.h>
#endif 

#include "MMString.h"
#include "cell/thread.h"
#include "LoadingScreen.h"
#include <Serialise.h>
#include <Explode.h>

#include "RPCS3.h"
#include "ppcasm.h"
#include "PoppetOutlineShapes.h"

#include "ResourceSystem.h"
#include <FartRO.h>
#include <resources/ResourceAnimatedTexture.h>

extern "C" void _gfxbind_hook_naked();
extern void InitGooeyNetworkHooks();
extern bool InitTweakSettings();
extern void LoadSackboyPolygon();
extern void LoadBallPolygon();
extern bool InitializeExplosiveStyles();
extern void AttachWebternateHooks();

bool AlearCheckPatch();

bool AlearEpilogue()
{
    DebugLog("FileDB::DBs:\n");
    {
        CCSLock _the_lock(&FileDB::Mutex, __FILE__, __LINE__);
        for (int i = 0; i < FileDB::DBs.size(); ++i)
        {
            // skip sync database
            if (i == 1) continue;

            gFileWatcher->AddFile(FileDB::DBs[i]->Path, OnDatabaseFileChanged);
            DebugLog("\t[0x%x]: %s\n", (u32)FileDB::DBs[i], FileDB::DBs[i]->Path.c_str());
        }
    }

    LoadCursorSprites();
    LoadRecordingShaders();
    LoadPostProcessingShaders();
    LoadSackboyPolygon();
    LoadBallPolygon();
    InitializeExplosiveStyles();
    
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

    FileDB::DBs.push_back(&sync::Database);
    sync::Database.Path = CFilePath(FPR_GAMEDATA, "gamedata/alear/sync/cache.map");
    
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

#include <gooey/GooeyImage.h>

extern MAKE_THREAD_FUNCTION(MainLoadingThread);
void AlearStartup()
{
    DebugLog("Alear version v%f build date: " __DATE__ " time: " __TIME__ "\n", mmalex::sqrtf(ALEAR_VERSION));

    DebugLog("Injecting init steps into startup...\n");
    DebugLog("First CInitStep: %s\n", gInitSteps[0].DebugText);
    DebugLog("First ps3test1 CInitStep: %s\n", gPs3Test1InitSteps[0].DebugText);
    DebugLog("First Alear CInitStep: %s\n", gAlearInitSteps[0].DebugText);
    DebugLog("SPRX TOC Base: %x\n", gTocBase);

    DebugLog("sizeof(CGooeyImage) = 0x%08x\n", sizeof(CGooeyImage));

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

    MH_PokeHook(0x0040b678, SetJetpackTether);
    MH_PokeHook(0x0040a9f4, CollectGun);
    MH_PokeHook(0x0040a92c, SetScubaGear);

    MH_PokeHook(0x00090538, MainLoadingThread);

    MH_Poke32(0x0001da24, 0x39200001);
    

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