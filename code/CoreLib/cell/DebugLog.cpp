#include "cell/DebugLog.h"

#include "printf.h"
#include <sys/tty.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

DebugChannelOptions g_DebugChannelMapping[] = {
    { 0, true, "DC_STDOUT", NULL },
    { 4, true, "DC_DEFAULT", NULL },
    { 4, true, "DC_SYSTEM", "System" },
    { 5, true, "DC_RESOURCE", "Resource" },
    { 6, true, "DC_PLAYER_PROFILE", "PlayerProfile" },
    { 7, true, "DC_NETWORK", "Network" },
    { 8, true, "DC_SCRIPT", "Script" },
    { 9, true, "DC_AC", "AC" },
    { 10, true, "DC_PUBLISH", "Publish" },
    { 11, true, "DC_HTTP", "HTTP" },
    { 12, true, "DC_GRAPHICS", "Graphics" },
    { 13, true, "DC_WEBCAM", "Webcam" },
    { 14, true, "DC_LOCALISATION", "Localisation" },
    { 15, true, "DC_INIT", "Init" },
    { 4, true, "DC_BUILD", "Build" },
    { 4, true, "DC_VOIP", "VOIP" },
    { 3, true, "DC_TESTSUITE", "TestSuite" },
    { 12, true, "DC_REPLAY", "Replay" },
    { 4, true, "DC_ALEAR", "Alear" },
    #ifdef SCRIPT_DEBUG
    { 4, true, "DC_SCRIPT_DEBUG", "ScriptDebug" },
    #else
    { 4, false, "DC_SCRIPT_DEBUG", "ScriptDebug" },
    #endif 
    { 4, true, "DC_SM64", "SM64" },
    { 4, true, "DC_ALEARSYNC", "AlearSync" }
};

void DebugLog(const char* format, ...) {
    va_list args;
    va_start(args, format);
    DebugLogChV(DC_ALEAR, format, args);
    va_end(args);
}

void DebugLogChR(EDebugChannel channel, char* b, char* e) 
{
    DebugChannelOptions* opt = g_DebugChannelMapping + channel;
    if (!opt->Enabled) return;
    
    unsigned int len;
    if (opt->Name != NULL)
    {
        char prefix[32];
        snprintf(prefix, 32, "[%s] ", opt->Name);
        sys_tty_write(0, prefix, strlen(prefix), &len);
    }

    sys_tty_write(0, b, e - b, &len);
}

void DebugLogChF(EDebugChannel channel, const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    DebugLogChV(channel, format, args);
    va_end(args);
}

void DebugLogChV(EDebugChannel channel, const char* format, va_list args) 
{
    if (!g_DebugChannelMapping[channel].Enabled) return;

    char buffer[520];
    vsnprintf(buffer, 512, format, args);
    DebugLogChR(channel, buffer, buffer + strlen(buffer));
}

void DebugLogEnable(EDebugChannel channel, bool enable)
{
    g_DebugChannelMapping[channel].Enabled = enable;
}

const char* gNewLine = "\n";

extern "C" void DEBUG_PRINT(const char* format, ...)
{
    // va_list args;
    // va_start(args, format);
    // DebugLogChV(DC_SM64, format, args);
    // va_end(args);
    
    // unsigned int len;
    // sys_tty_write(0, gNewLine, 1, &len);
}