#include "AlearDebugCamera.h"
#include "AlearSync.h"
#include "AlearConfig.h"
#include "PinSystem.h"

#include <cell/fs/cell_fs_file_api.h>
#include <cell/gcm.h>

#include <hook.h>
#include <mmalex.h>
#include <ppcasm.h>
#include <json_ext.h>
#include <printf.h>
#include <filepath.h>

#include <gfxcore.h>
#include <padinput.h>
#include <cell/DebugLog.h>

#include <GFXApi.h>
#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <GuidHashMap.h>
#include <thing.h>
#include <PartYellowHead.h>
#include <PartPhysicsWorld.h>
#include <Poppet.h>
#include <ResourceGFXFont.h>
#include <View.h>
#include <OverlayUI.h>
#include <LoadingScreen.h>
#include <gooey/GooeyNodeManager.h>
#include <network/NetworkManager.h>
#include <gooey/GooeyScriptInterface.h>


#ifdef __SM64__
extern void SpawnMarioAvatar(EPadIndex pad, float x, float y, float z);
extern void ClearMarioAvatars();
extern bool ApplyMainAvatarCamera(CCamera* camera);
#endif 

enum ECameraMenu {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_TIMELINE,
    MENU_PLAYBACK
};

ECameraMenu gCameraMenu = MENU_MAIN;
bool gShowCameraMenu = false;
bool gShowCameraInfo = false;
bool gUseLegacyDebugCamera = false;
bool gDisableCameraInput = false;
bool gDisableDOF = true;
bool gDisableFog = false;
bool gShowOutlines = false;
bool gRenderOnlyPopit = false;


CGooeyNodeManager* gCameraGooey;

void UpdateDebugCameraNotInUse()
{
    #ifdef __CINEMACHINE__
    if (gCinemachine.IsPlaying())
        gCinemachine.Stop();
    #endif
    
    #ifdef __SM64__
    ClearMarioAvatars();
    #endif

    gDisableCameraInput = false;
    gShowCameraMenu = false;
}

float UnpackAnalogue(u16 raw)
{
    float val = (int)(raw - 128);
    if (abs(val) < 37.0f) return 0.0f;
    return val / 128.0f;
}

void HandleCameraMovementInput(CView* view)
{
    if (gDisableCameraInput) return;

    float lx = UnpackAnalogue(gPadData->LeftStickX);
    float ly = UnpackAnalogue(gPadData->LeftStickY);
    float rx = UnpackAnalogue(gPadData->RightStickX);
    float ry = UnpackAnalogue(gPadData->RightStickY);
    float l2 = gPadData->PressureData[PAD_PRESSURE_L2] / 255.0f;
    float r2 = gPadData->PressureData[PAD_PRESSURE_R2] / 255.0f;

    CDebugCamera& camera = view->DebugCamera;
    camera.Yaw += ry * 0.1f;
    camera.Pitch += rx * 0.1f;

    m44 inv_camrot = m44::rotationZYX(v3(-camera.Yaw, -camera.Pitch, 0.0f));

    v4 delta(lx * 100.0f, -ly * 100.0f, (l2 + -r2) * 100.0f, 1.0f);
    delta = inv_camrot * delta;

    camera.Pos += delta;
    camera.Foc = camera.Pos - (inv_camrot * v4(0.0f, 0.0f, 3000.0f, 1.0f));
}

void UpdateCameraUI(CGooeyNodeManager* manager)
{
    manager->SetStylesheetScalingFactor(0.5f);
    if (!manager->StartFrameNamed(1000)) return;

    manager->SetFrameSizing(gResX, gResY);
    manager->SetFrameLayoutMode(LM_CENTERED, LM_CENTERED);
    if (manager->StartFrame())
    {
        manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
        manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Contents());
        manager->SetFrameBorders(32.0f, 32.0f);
        manager->SetFrameDefaultChildSpacing(0.0f, 0.0f);

        // manager->DoText(L"Debug Camera", GTS_T2);
        // manager->DoHorizontalBreak(GBS_SOLID, v2(0.5f, 0.5f));

        if (manager->StartFrame())
        {
            manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_END);
            manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Contents());
            manager->SetFrameBorders(0.0f, 0.0f);
            manager->SetFrameDefaultChildSpacing(32.0f, 16.0f);

            switch (gCameraMenu)
            {
                case MENU_MAIN:
                {
                    #ifdef __CINEMACHINE__
                    if (manager->DoInline(L"Cinemachine", GTS_SMALL_PRINT, STATE_NORMAL, NULL, 256) & 256)
                        gCameraMenu = MENU_TIMELINE;
                    manager->DoBreak();
                    #endif
                    
                    if (manager->DoInline(L"Settings", GTS_SMALL_PRINT, STATE_NORMAL, NULL, 256) & 256)
                        gCameraMenu = MENU_SETTINGS;


                    #ifdef __SM64__
                    manager->DoBreak();

                    if (manager->DoInline(L"Mario", GTS_SMALL_PRINT, STATE_NORMAL, NULL, 256) & 256)
                    {
                        gShowCameraMenu = false;
                        gDisableCameraInput = true;
                        EPlayerNumber leader = gNetworkManager.InputManager.GetLocalLeadersPlayerNumber();
                        CThing* player = gGame->GetYellowheadFromPlayerNumber(leader);
                        if (player != NULL && player->GetPYellowHead() != NULL)
                        {
                            v4 pos = player->GetPYellowHead()->GetActivePosition();
                            SpawnMarioAvatar(E_PAD_INDEX_0, (float)pos.getX(), (float)pos.getY() + 90.0f, (float)pos.getZ());
                        }
                    }
                    #endif

                    break;
                }
                case MENU_SETTINGS:
                {
                    #define INLINE_BOOL(name, variable) \
                    if (manager->DoInline(L##name, GTS_SMALL_PRINT, variable ? STATE_TOGGLE : STATE_NORMAL, NULL, 256) & 256) \
                        variable = !variable; \
                    manager->DoBreak();
                    

                    INLINE_BOOL("Disable DOF", gDisableDOF);
                    INLINE_BOOL("Disable Fog", gDisableFog);
                    INLINE_BOOL("Show Outlines", gShowOutlines);
                    INLINE_BOOL("Show Popit Only", gRenderOnlyPopit);
                    INLINE_BOOL("Show Extra Info", gShowCameraInfo);
                    INLINE_BOOL("Pause Game Simulation", gPauseGameSim);

                    #undef INLINE_BOOL
                    
                    break;
                }

                #ifdef __CINEMACHINE__
                case MENU_TIMELINE:
                {
                    for (int i = 0; i < gClips.size(); ++i)
                    {
                        CCameraClip* clip = gClips[i];
                        bool active = gCinemachine.IsPlaying() && gCinemachine.GetActiveClip() == clip;
                        if (manager->DoInline(clip->Name.c_str(), GTS_SMALL_PRINT, active ? STATE_TOGGLE : STATE_NORMAL, NULL, 256) & 256)
                        {
                            gCameraMenu = MENU_PLAYBACK;
                            gCinemachine.Play(clip);
                        }

                        manager->DoBreak();
                    }

                    if (manager->DoButton(L"Reload From Disk", GTS_SMALL_PRINT, STATE_NORMAL, NULL, 256) & 256)
                        LoadCameraClips();

                    break;
                }

                case MENU_PLAYBACK:
                {
                    // input doesn't work because we're not rendering anything
                    if ((gPadData->ButtonsDown & PAD_BUTTON_CIRCLE) != 0)
                    {
                        gCinemachine.Stop();
                        gCameraMenu = MENU_TIMELINE;
                    }
                    // Position
                    // Transition Type
                    // Timing
                        // Transition Time
                        // Hold Time
                    // Focus
                        // Field of View
                        // Depth of Field
                    break;
                }
                #endif
            }


            manager->EndFrame();
        }


        manager->EndFrame();
    }

    if ((manager->EndFrame(512) & 512) != 0)
    {
        if (gCameraMenu == MENU_MAIN) gShowCameraMenu = false;

        #ifdef __CINEMACHINE__
        else if (gCameraMenu == MENU_PLAYBACK)
        {
            gCinemachine.Stop();
            gCameraMenu = MENU_TIMELINE;
        }
        else if (gCameraMenu == MENU_TIMELINE)
        {
            if (gCinemachine.IsPlaying())
                gCinemachine.Stop();
            else
                gCameraMenu = MENU_MAIN;
        }
        #endif

        else gCameraMenu = MENU_MAIN;
    }
}

void UpdateDebugCamera()
{
    CView* view = &gView;
    if (gUseLegacyDebugCamera || gView.MaverickCam) return;

    if (gShowCameraMenu)
    {
        if (gCameraGooey == NULL)
            gCameraGooey = new CGooeyNodeManager(INPUT_ALL, E_GOOEY_MANAGER_NETWORK_NONE);

        UpdateCameraUI(gCameraGooey);
        gCameraGooey->PerFrameUpdate(E_PLAYER_NUMBER_NONE, MODE_NORMAL, 0);
    }
    else HandleCameraMovementInput(view);

    #if __CINEMACHINE__
    gCinemachine.Update();
    #endif

    if (!gDisableCameraInput)
    {
        if ((gPadData->ButtonsDown & PAD_BUTTON_START) != 0)
        {
            #ifdef __CINEMACHINE__
            gCinemachine.Stop();
            #endif
            
            gShowCameraMenu = !gShowCameraMenu;
            gCameraMenu = MENU_MAIN;
        }

        if ((gPadData->ButtonsDown & PAD_BUTTON_TRIANGLE) != 0)
        {
            view->DebugCamera.Foc = view->DebugCamera.Pos - v4(0.0f, 0.0f, 3000.0f, 1.0f);
            view->DebugCamera.Yaw = 0.0f;
            view->DebugCamera.Pitch = 0.0f;
        }
    }
}

void OnUpdateDebugCameraForRender(CView* view)
{
    if (gUseLegacyDebugCamera || view->MaverickCam)
    {
        view->DebugCamera.Update(view->MaverickCam);
        view->DebugCamera.Apply(view->Camera);
        return;
    }

    #ifdef __CINEMACHINE__
        #ifdef __SM64__
            if (!ApplyMainAvatarCamera(view->Camera))
            {
                if (gCinemachine.IsPlaying())
                    gCinemachine.Apply(view->Camera);
                else 
                    view->DebugCamera.Apply(view->Camera);
            }
        #else
            if (gCinemachine.IsPlaying())
                gCinemachine.Apply(view->Camera);
            else 
                view->DebugCamera.Apply(view->Camera);
        #endif
    #else
        #ifdef __SM64__
            if (!ApplyMainAvatarCamera(view->Camera))
                view->DebugCamera.Apply(view->Camera);
        #else
            view->DebugCamera.Apply(view->Camera);
        #endif
    #endif

    if (gDisableDOF)
    {
        gView.DofMax = 0.0f;
        gView.DofMin = 0.0f;
    }


    gView.FogMin = 0.0f;
    gView.FogMax = 0.0f;   
    if (!gDisableFog)
    {
        const float fog_bias = 0.0f;
        PLevelSettings* settings = GGetLevelSettings(gGame->GetWorld());
        if (settings != NULL)
        {
            if (settings->FogNear <= settings->FogFar)
            {
                gView.FogMax = settings->FogFar + fog_bias;
                gView.FogMin = fog_bias + settings->FogNear;
            }
        }
    }
}

MMString<wchar_t> gCameraFocString;
MMString<wchar_t> gCameraPosString;
MMString<wchar_t> gWorldListString;
MMString<wchar_t> gWorldStrings[64];

extern CRawVector<PWorld*> gWorlds;

v4 GetCameraPosition()
{
    #ifdef __CINEMACHINE__
    return gCinemachine.IsPlaying() ? gCinemachine.Camera.Pos : gView.DebugCamera.Pos;
    #else
    return gView.DebugCamera.Pos;
    #endif
}

v4 GetCameraFocus()
{
    #ifdef __CINEMACHINE__
    return gCinemachine.IsPlaying() ? gCinemachine.Camera.Foc : gView.DebugCamera.Foc;
    #else
    return gView.DebugCamera.Foc;
    #endif
}

void OnDrawPostComp(COverlayUI* interface)
{
    RenderDownloadInfo();
    RenderPinOverlay();

    if (!gView.DebugCameraActive && !gRenderOnlyPopit)
    {
        UpdateButtonPrompts();
        return;
    }

    if (gCameraMenu == MENU_PLAYBACK) return;

    if (gShowCameraInfo)
    {
        #ifdef __CINEMACHINE__
        v4 camera_pos = gCinemachine.IsPlaying() ? gCinemachine.Camera.Pos : gView.DebugCamera.Pos;
        v4 camera_foc = gCinemachine.IsPlaying() ? gCinemachine.Camera.Foc : gView.DebugCamera.Foc;
        #else
        v4 camera_pos = gView.DebugCamera.Pos;
        v4 camera_foc = gView.DebugCamera.Foc;
        #endif

        char fmt[1024];
        sprintf(fmt, "Position: <%f, %f, %f>", (float)camera_pos.getX(), (float)camera_pos.getY(), (float)camera_pos.getZ());
        const tchar_t* pos_str = (const tchar_t*)MultiByteToWChar(gCameraPosString, fmt, NULL);
        sprintf(fmt, "Focus:    <%f, %f, %f>", (float)camera_foc.getX(), (float)camera_foc.getY(), (float)camera_foc.getZ());
        const tchar_t* foc_str = (const tchar_t*)MultiByteToWChar(gCameraFocString, fmt, NULL);

        const u32 text_color = 0xFFFFFFFF;
        StartDrawText(true, NULL, NULL, false, true, gResX, gResY);
        DrawText(gFont[FONT_DEFAULT], pos_str, 16.0f, 21.0f, 0.0f, 7.0f, text_color, -1.0f);
        DrawText(gFont[FONT_DEFAULT], foc_str, 16.0f, 35.0f, 0.0f, 7.0f, text_color, -1.0f);


        sprintf(fmt, "Worlds [%d]:", gWorlds.size());
        const tchar_t* worldlist_str = (const tchar_t*)MultiByteToWChar(gWorldListString, fmt, NULL);
        DrawText(gFont[FONT_DEFAULT], worldlist_str, 16.0f, 49.0f, 0.0f, 7.0f, text_color, -1.0f);

        for (int i = 0; i < gWorlds.size(); ++i)
        {
            PWorld* world = gWorlds[i];
            RLevel* level = world->Level;

            if (level == NULL)
            {
                sprintf(fmt, "[%d]: No attached level", i);
                const tchar_t* world_str = (const tchar_t*)MultiByteToWChar(gWorldStrings[i], fmt, NULL);
                DrawText(gFont[FONT_DEFAULT], world_str, 32.0f, 49.0f + ((i + 1) * 14.0f), 0.0f, 7.0f, text_color, -1.0f);
            }
            else
            {
                CGUID guid = level->GetGUID();
                CFileDBRow* row = FileDB::FindByGUID(guid);
                if (row != NULL)
                {
                    sprintf(fmt, "[%d]: %s (g%d)", i, row->FilePathX, level->GetGUID().guid);
                    const tchar_t* world_str = (const tchar_t*)MultiByteToWChar(gWorldStrings[i], fmt, NULL);
                    DrawText(gFont[FONT_DEFAULT], world_str, 32.0f, 49.0f + ((i + 1) * 14.0f), 0.0f, 7.0f, text_color, -1.0f);
                }
                else
                {
                    sprintf(fmt, "[%d]: g%d", i, level->GetGUID().guid);
                    const tchar_t* world_str = (const tchar_t*)MultiByteToWChar(gWorldStrings[i], fmt, NULL);
                    DrawText(gFont[FONT_DEFAULT], world_str, 32.0f, 49.0f + ((i + 1) * 14.0f), 0.0f, 7.0f, text_color, -1.0f);
                }
            }
        }


        EndDrawText();
    }

    if (!gShowCameraMenu || gCameraGooey == NULL) return;

    gCameraGooey->RenderToTexture(gResX, gResY, gResX, gResY, false, false);
}

extern "C" void _alearcam_update_hook();
extern "C" void _alearcam_renderworld_hook();
void InitCameraHooks()
{
    MH_Poke32(0x00014acc, B(&_alearcam_update_hook, 0x00014acc));
    MH_Poke32(0x001e6b8c, B(&_alearcam_renderworld_hook, 0x001e6b8c));
    MH_InitHook((void*)0x002716fc, (void*)&OnDrawPostComp);
    MH_InitHook((void*)0x001fc920, (void*)&OnUpdateDebugCameraForRender);

    MH_Poke32(0x00014b2c, 0x540003de); // change teleport to select
}

#ifdef __CINEMACHINE__

CVector<CCameraClip*> gClips(16);
CCinemachine gCinemachine;

bool LoadCameraClip(char* json)
{
    const int MAX_FIELDS = 4096;
    json_t pool[MAX_FIELDS];
    json_t const* root = json_create(json, pool, MAX_FIELDS);
    if (root == NULL)
    {
        DebugLog("Failed to load camera clip, JSON was invalid!\n");
        return false;
    }

    if (json_getType(root) != JSON_OBJ)
    {
        DebugLog("Root of camera clip definition must be a JSON object!\n");
        return false;
    }

    CCameraClip* clip = new CCameraClip();
    const json_t* prop = NULL;


    const char* name = json_getPropertyValue(root, "name");
    if (name == NULL) name = "Unnamed Clip";
    MultiByteToWChar(clip->Name, name, NULL);

    prop = json_getProperty(root, "looped");
    if (prop != NULL && json_getType(prop) == JSON_BOOLEAN)
        clip->IsLooping = json_getBoolean(prop);

    const json_t* json_shots = json_getProperty(root, "shots");
    if (json_shots != NULL)
    {
        if (json_getType(json_shots) != JSON_ARRAY)
        {
            DebugLog("Camera shot list must be a JSON array!\n");
            delete clip;
            return false;
        }

        for (json_t const* element = json_getChild(json_shots); element != NULL; element = json_getSibling(element))
        {
            if (json_getType(element) != JSON_OBJ)
            {
                DebugLog("Element in shot array is invalid! (Not a JSON object), failing load!\n");
                delete clip;
                return false;
            }

            CCameraKeyframe frame;

            const char* shot_type = json_getStringProperty(element, "shot_type", "CUT");
            const char* target_type = json_getStringProperty(element, "target_type", "POSITION");

            if (StringCompare(shot_type, "ORBIT_3D") == 0)
                frame.ShotType = SHOT_ORBIT_3D;
            else if (StringCompare(shot_type, "ORBIT_2D") == 0)
                frame.ShotType = SHOT_ORBIT_2D;
            else if (StringCompare(shot_type, "CUT") == 0)
                frame.ShotType = SHOT_CUT;
            else if (StringCompare(shot_type, "OFFSET") == 0)
                frame.ShotType = SHOT_OFFSET;
            else
            {
                DebugLog("Unknown camera shot type (%s), failing load!\n", shot_type);
                delete clip;
                return false;
            }

            if (StringCompare(target_type, "POSITION") == 0)
                frame.TargetType = TARGET_POSITION;
            else if (StringCompare(target_type, "PLAYER") == 0)
                frame.TargetType = TARGET_PLAYER;
            else if (StringCompare(target_type, "OBJECT") == 0)
                frame.TargetType = TARGET_OBJECT;
            else
            {
                DebugLog("Unknown camera target type (%s), failing load!\n", target_type);
            }

            frame.TransitionTime = json_getFloatProperty(element, "transition_time", 0.0f);
            frame.HoldTime = json_getFloatProperty(element, "hold_time", 0.0f);
            frame.Focus = json_getVectorProperty(element, "focus", v4(0.0f));
            frame.Position = json_getVectorProperty(element, "position", v4(0.0f));
            frame.Target = json_getIntProperty(element, "target", 0);

            json_t const* orbit = json_getProperty(element, "orbit");
            if (orbit != NULL)
            {
                if (json_getType(orbit) != JSON_OBJ)
                {
                    DebugLog("Orbit config has invalid object type! Expected JSON_OBJ, cancelling load!\n");
                    delete clip;
                    return false;
                }

                frame.Orbit.Distance = json_getFloatProperty(orbit, "distance", 400.0f);
                frame.Orbit.Radius = json_getFloatProperty(orbit, "radius", 200.0f);
                frame.Orbit.Period = json_getFloatProperty(orbit, "period", 1.0f);
                frame.Orbit.AngleOffset = json_getFloatProperty(orbit, "angle_offset", 0.0f);
                frame.Orbit.Flipped = json_getBoolProperty(orbit, "flipped", false);
            }

            clip->Keyframes.push_back(frame);
        }
    }

    gClips.push_back(clip);
    DebugLog("Successfully added %s to clip store!\n", name);
    return true;
}

bool LoadCameraClip(CFilePath& fp)
{
    char* json = FileLoadText(fp);
    if (json == NULL)
    {
        DebugLog("Failed to read file data for camera clip!\n");
        return false;
    }

    bool ret = LoadCameraClip(json);
    delete json;
    return ret;
}

bool LoadCameraClips()
{
    gCinemachine.Stop();
    for (int i = 0; i < gClips.size(); ++i)
    {
        CCameraClip* clip = gClips[i];
        delete clip;
    }
    gClips.clear();
    DebugLog("Starting load of camera config clips...\n");

    CFilePath clipdir(FPR_GAMEDATA, "gamedata/alear/clips");

    int fd;

    // Not going to treat it as an error if the directory fails to open,
    // since the clips are technically optional.
    if (cellFsOpendir(clipdir.c_str(), &fd) != CELL_FS_OK)
    {
        DebugLog("Not loading any clips because directory doesn't exist!\n");
        return true;
    }

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
        sprintf(buf, "%s/%s", clipdir.c_str(), entry.entry_name.d_name);
        fp.Assign(buf);
        if (!LoadCameraClip(fp))
            DebugLog("Failed to load camera clip from %s, an error occurred!\n", buf);
    } while (data_count);

    cellFsClosedir(fd);
    return true;
}

float CCameraKeyframe::GetOrbitAngle(float time)
{
    const float deg2rad = 0.01745329251f;
    float pct = mmalex::fmod(time / Orbit.Period, 1.0f);
    float deg = (Orbit.Flipped ? -1.0f : 1.0f) * (pct * 360.0f) + Orbit.AngleOffset;
    return deg * deg2rad;
}

SCameraData CCameraKeyframe::GetCameraData(float time)
{
    v4 pos, foc;

    switch (TargetType)
    {
        case TARGET_PLAYER:
        {
            EPlayerNumber leader = gNetworkManager.InputManager.GetLocalLeadersPlayerNumber();
            CThing* player = gGame->GetYellowheadFromPlayerNumber(leader);
            if (player != NULL && player->GetPYellowHead() != NULL)
                foc = player->GetPYellowHead()->GetActivePosition();
            
            break;
        }
        case TARGET_OBJECT:
        {
            RLevel* level = gGame->Level;
            if (level == NULL) break;
            CThing* thing = level->WorldThing;
            if (thing == NULL) break;
            PWorld* world = thing->GetPWorld();
            if (world == NULL) break;

            CThing* target_thing = world->GetThingByUID(Target);
            if (target_thing != NULL && target_thing->GetPPos() != NULL)
                foc = target_thing->GetPPos()->GetBestGameplayPosv4();

            break;
        }
        case TARGET_POSITION:
        default:
        {
            foc = Focus;
            break;
        }
    }

    switch (ShotType)
    {
        case SHOT_CUT:
        default:
        {
            pos = Position;
            break;
        }

        case SHOT_OFFSET:
        {
            pos = Position + foc;
            break;
        }

        case SHOT_ORBIT_2D:
        {
            float angle = GetOrbitAngle(time);
            pos = foc + v4(Orbit.Radius * mmalex::cos(angle), Orbit.Radius * mmalex::sin(angle), Orbit.Distance, 0.0f);
            break;
        }

        case SHOT_ORBIT_3D:
        {
            float angle = GetOrbitAngle(time);
            pos = foc + v4(Orbit.Radius * mmalex::cos(angle), 0.0f, Orbit.Radius * mmalex::sin(angle), 0.0f);
            break;
        }
    }

    // Fix up the W component just in case
    // for matrix math, oops
    pos.setW(1.0f);
    foc.setW(1.0f);

    return SCameraData(pos, foc);
}

void CCinemachine::Play(CCameraClip* clip)
{
    Stop();

    if (clip == NULL || clip->Keyframes.size() == 0) return;

    ActiveClip = clip;
    State = STATE_PLAYING;
}

void CCinemachine::Stop()
{
    Frame = 0;
    StateTimer = 0.0f;
    ActiveClip = NULL;
    State = STATE_IDLE;
}

const char* gStateNames[] = { "STATE_IDLE", "STATE_PLAYING", "STATE_TRANSITION" };

void CCinemachine::Update()
{
    if (State == STATE_IDLE) return;

    // DebugLog("[CCinemachine] [STATE]=%s, [TIMER]=%f, [FRAME]=%d\n", gStateNames[State], StateTimer, Frame);

    const float delta = 1.0f / 30.0f; // hehehehehehe
    float time = StateTimer;
    StateTimer += delta;

    CCameraKeyframe& frame = ActiveClip->Keyframes[Frame];
    switch (State)
    {
        case STATE_PLAYING:
        {
            if (time >= frame.HoldTime)
                time = frame.HoldTime;

            SCameraData cam = frame.GetCameraData(time);
            gCinemachine.Apply(cam);

            if (time >= frame.HoldTime)
            {
                if (frame.HasTransition() && HasFramesRemaining())
                {
                    SetState(STATE_TRANSITION);
                    LerpStart = cam;
                    LerpTarget = GetNextFrame()->GetCameraData(0.0f);
                }
                else NextFrame();
            }

            break;
        }
        case STATE_TRANSITION:
        {
            float factor = time / frame.TransitionTime;
            gCinemachine.Apply(LerpStart, LerpTarget, factor);
            if (time >= frame.TransitionTime) NextFrame();
            break;
        }
    }
}

#endif