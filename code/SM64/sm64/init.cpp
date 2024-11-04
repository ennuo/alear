#include <surface_terrains.h>

#include "sm64/init.h"
#include "sm64/level.h"

#include <filepath.h>
#include <padinput.h>
#include <cell/DebugLog.h>

#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <PartRenderPosition.h>
#include <PartPhysicsWorld.h>

bool InitMarioLib()
{
    CFilePath fp(FPR_GAMEDATA, "gamedata/alear/sm64/baserom.us.z64");
    if (!FileExists(fp))
    {
        DebugLog("No ROM for SM64 exists at %s!\n", fp.c_str());
        return false;
    }

    // lazy just use the text version
    u8* rom = (u8*)FileLoadText(fp);
    if (rom == NULL)
    {
        DebugLog("Failed to read ROM file at %s!\n", fp.c_str());
        return false;
    }

    u8* texture = new u8[4 * SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT];

    sm64_global_init(rom, texture);
    //sm64_audio_init(rom); // crashes figure out why later
    sm64_static_surfaces_load(gTestSurfaces, gTestSurfaceCount);

    delete rom;
    delete[] texture;

    return true;
}

void CloseMarioLib()
{
    //sm64_stop_background_music(sm64_get_current_background_music());
    sm64_global_terminate();
}

void ClearMarioAvatars()
{
    if (gMarioAvatars.size() == 0) return;
    CMarioAvatar** it = gMarioAvatars.begin();
    for (; it != gMarioAvatars.end(); ++it)
        delete *it;
    gMarioAvatars.clear();
}

void UpdateMarioAvatars()
{
    CMarioAvatar** it = gMarioAvatars.begin();
    for (; it != gMarioAvatars.end(); ++it)
        (*it)->Tick();
}

void SpawnMarioAvatar(EPadIndex pad, float x, float y, float z)
{
    CMarioAvatar* avatar = new CMarioAvatar(pad, x, y, z);
    gMarioAvatars.push_back(avatar);
}

bool ApplyMainAvatarCamera(CCamera* camera)
{
    if (gMarioAvatars.size() == 0) return false;
    CMarioAvatar* avatar = gMarioAvatars[0];
    avatar->ApplyCamera(camera);
    return true;
}