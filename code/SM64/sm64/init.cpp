#include <surface_terrains.h>

#include "sm64/init.h"
#include "sm64/level.h"

#include <filepath.h>
#include <padinput.h>
#include <cell/DebugLog.h>
#include <CritSec.h>

#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <PartRenderPosition.h>
#include <PartPhysicsWorld.h>

#include <cell/thread.h>




struct DualSample16 { s16 L; s16 R; };
#define RB_MAX_SIZE (1000000)
char* gReadBuffer;
int gReadBufferSize = 0;

CCriticalSec* gAudioCS;

MH_DefineFunc(FMOD_System_createSound, 0x004aede4, TOC1, FMOD_RESULT, FMOD::System*, const char*, FMOD_MODE, FMOD_CREATESOUNDEXINFO*, FMOD::Sound**);
FMOD_RESULT F_API FMOD::System::createSound(const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD::Sound **sound)
{
    return FMOD_System_createSound(this, name_or_data, mode, exinfo, sound);
}

MH_DefineFunc(FMOD_System_playSound, 0x004aeb14, TOC1, FMOD_RESULT, FMOD::System*, FMOD_CHANNELINDEX, FMOD::Sound*, bool, FMOD::Channel**);
FMOD_RESULT F_API FMOD::System::playSound(FMOD_CHANNELINDEX channelid, FMOD::Sound *sound, bool paused, FMOD::Channel **channel)
{
    return FMOD_System_playSound(this, channelid, sound, paused, channel);
}

FMOD_RESULT F_CALLBACK OnReadMarioAudio(FMOD_SOUND* sound, void* data, unsigned int datalen)
{
    if (gReadBufferSize == 0) return FMOD_OK;

    int len = MIN(datalen, gReadBufferSize);
    memcpy(data, gReadBuffer, len);

    gAudioCS->Enter(__FILE__, __LINE__);
    gReadBufferSize -= len;
    memmove(gReadBuffer, gReadBuffer + len, gReadBufferSize);
    gAudioCS->Leave();


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK OnSetMarioAudioPos(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    return FMOD_OK;
}

FMOD::Sound* gSound;
bool gStartedSound = false;


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
    sm64_audio_init(rom); // crashes figure out why later
    sm64_static_surfaces_load(gTestSurfaces, gTestSurfaceCount);
    // sm64_play_music(0, 0x05 | 0x80, 0);

    delete rom;
    delete[] texture;

    gAudioCS = new CCriticalSec("MyAn");
    gReadBuffer = new char[RB_MAX_SIZE];

    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.decodebuffersize = 512 * sizeof(DualSample16);
    exinfo.length = 32000 * sizeof(DualSample16);
    exinfo.numchannels = 2;
    exinfo.defaultfrequency = 32000;
    exinfo.format = FMOD_SOUND_FORMAT_PCM16;
    exinfo.pcmreadcallback = OnReadMarioAudio;

    FMOD_RESULT res = CAudio::System->createSound(NULL, FMOD_LOOP_NORMAL | FMOD_OPENUSER | FMOD_CREATESTREAM | FMOD_2D, &exinfo, &gSound);
    if (res != FMOD_OK)
    {
        DebugLogChF(DC_SM64, "Failed to create FMOD handle for SM64 audio!\n");
        return false;
    }



    return true;
}

void CloseMarioLib()
{
    sm64_stop_background_music(sm64_get_current_background_music());
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

    // gAudioCS->Enter(__FILE__, __LINE__);
    // int num_samples = sm64_audio_tick(gReadBufferSize / sizeof(DualSample16), 1100, (s16*)(gReadBuffer + gReadBufferSize));
    // gReadBufferSize += num_samples * 2 * sizeof(DualSample16);
    // gAudioCS->Leave();

    if (!gStartedSound)
    {
        FMOD::Channel* channel;
        CAudio::System->playSound(FMOD_CHANNEL_FREE, gSound, false, &channel);
        gStartedSound = true;
    }

}

void UpdateMarioAvatarsRender()
{
    CMarioAvatar** it = gMarioAvatars.begin();
    for (; it != gMarioAvatars.end(); ++it)
        (*it)->UpdateThing();
}

void UpdateMarioDebugRender()
{
    CMarioAvatar** it = gMarioAvatars.begin();
    for (; it != gMarioAvatars.end(); ++it)
        (*it)->DoDebugRender();
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