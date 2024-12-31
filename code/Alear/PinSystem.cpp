#include "PinSystem.h"
#include "resources/ResourcePins.h"

#include <PlayerMetaData.h>
#include <TextRange.h>
#include <MMString.h>
#include <cell/DebugLog.h>

#include <hook.h>
#include <ppcasm.h>

CGetPinsTask gPinsTask;

void CGetPinsTask::StartTask()
{
    AuthenticatedTask.StopTask();
    AuthenticatedTask.MakeHTTPRequest("http://refresh.jvyden.xyz:2095/lbp/get_my_pins", NULL, E_HTTP_GET, E_HTTP_PRIORITY_HIGH, "application/json", E_HTTP_DIGEST_NONE, false);
    CHTTPResponseArray* response = new CHTTPResponseArray();
    *((uint32_t*)response) = 0x0090c598;
    AuthenticatedTask.StartTask(response, NULL);
}

void CGetPinsTask::Update()
{
    EHTTPTaskStatus status = AuthenticatedTask.GetTaskStatus();
    if (status == E_HTTP_TASK_FAIL)
    {
        DebugLogChF(DC_NETWORK, "NET: We failed our get pin request!\n");
        AuthenticatedTask.StopTask();
        return;
    }

    if (status == E_HTTP_TASK_INIT && !Valid)
    {
        DebugLog("STARTING PINS TASK!!!\n");
        StartTask();
        Valid = true;
    }


}


MH_DefineFunc(SplitTokens, 0x0015d69c, TOC0, void, TextRange<char> range, char split_char, CVector<MMString<char> >& tokens);
MH_DefineFunc(TryParseU32Value, 0x00582ed8, TOC1, bool, TextRange<char> range, u32* result_int);

bool ParsePins(TextRange<char> range, u32 pins[MAX_PINS])
{
    memset(pins, 0, sizeof(pins));
    CVector<MMString<char> > tokens;
    SplitTokens(range, ',', tokens);
    int len = MIN(tokens.size(), MAX_PINS);
    for (int i = 0; i < len; ++i)
        TryParseU32Value(tokens[i].c_str(), &pins[i]);

    DebugLog("Parsed pins response: [0x%08x, 0x%08x, 0x%08x]\n", pins[0], pins[1], pins[2]);

    return true;
}

extern "C" uintptr_t _pin_parse_player_metadata_hook;
extern "C" uintptr_t _pin_update_http_tasks;
void InitPinHooks()
{
    MH_Poke32(0x001203a8, B(&_pin_update_http_tasks, 0x001203a8));
    MH_Poke32(0x001612fc, B(&_pin_parse_player_metadata_hook, 0x001612fc));
}