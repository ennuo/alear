#pragma once

const u32 MAX_MMLOGCH_STRING_LENGTH = 0x200;
extern bool gDebugLoggingEnabled;

enum EDebugChannel 
{
    DC_STDOUT,
    DC_DEFAULT,
    DC_SYSTEM,
    DC_RESOURCE,
    DC_PLAYER_PROFILE,
    DC_NETWORK,
    DC_SCRIPT,
    DC_AC,
    DC_PUBLISH,
    DC_HTTP,
    DC_GRAPHICS,
    DC_WEBCAM,
    DC_LOCALISATION,
    DC_INIT,
    DC_BUILD,
    DC_VOIP,
    DC_TESTSUITE,
    DC_REPLAY,
    DC_ALEAR,
    DC_SCRIPT_DEBUG,
    DC_SM64,
    DC_MOLD,

    NUM_DEBUG_CHANNELS
};

struct DebugChannelOptions
{
    u32 TTY;
    bool Enabled;
    const char* INI;

    // This isn't a real field, but I'm using it since retail builds
    // don't actually have this struct anyway.
    const char* Name;
};

void DebugLog(const char* format, ...);
extern "C" void DEBUG_PRINT(const char* format, ...);
extern DebugChannelOptions g_DebugChannelMapping[];

void DebugLogChR(EDebugChannel channel, char* b, char* e);
void DebugLogChF(EDebugChannel channel, const char* format, ...);
void DebugLogChV(EDebugChannel channel, const char* format, va_list args);
void DebugLogEnable(EDebugChannel channel, bool enable);

#define MMLogCh DebugLogChF
#define MMLog(...) DebugLogChF(DC_STDOUT, __VA_ARGS__)
