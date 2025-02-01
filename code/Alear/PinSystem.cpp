#include "PinSystem.h"
#include "resources/ResourcePins.h"

#include <PlayerMetaData.h>
#include <TextRange.h>
#include <MMString.h>
#include <cell/DebugLog.h>

#include <ResourceGFXTexture.h>
#include <ResourceGFXFont.h>
#include <ResourceSystem.h>
#include <MMAudio.h>


#include <hook.h>
#include <ppcasm.h>

template <typename T>
class CBouncer {};

template <>
class CBouncer<float> {
public:
    CBouncer() : Target(), P(), OP(), 
    k(0.25f), damp(0.7f)
    {}

    CBouncer(float target) : Target(target), P(target), OP(target), 
    k(0.25f), damp(0.7f)
    {}
public:
    float GetCurrent() { return P; }

    void Update()
    {
        float op = P;
        P += ((P - OP) * damp) + ((Target - P) * k);
        OP = op;
    }

    void Set(float target)
    {
        OP = target;
        P = OP;
        Target = P;
    }

    void SetTarget(float target) { Target = target; }
    void Kick(float kick) { OP -= kick; }
private:
    float Target;
    float P;
    float OP;
    float k;
    float damp;
};

class CPinsOverlay {
public:
    CPinsOverlay() : OmnesBlack(), BackgroundTexture(), 
    ShowTime(-1.0f), BackgroundBouncerScale(0.0f), 
    Colour(1.0f), Icon(), Text()
    {
        OmnesBlack = LoadResourceByKey<RFontFace>(16710, 0, STREAM_PRIORITY_DEFAULT);
        BackgroundTexture = LoadResourceByKey<RTexture>(105229, 0, STREAM_PRIORITY_DEFAULT);
    }
public:
    void StartShowPinReward(int pin_type)
    {
        ShowTime = 0.0f;
        BackgroundBouncerScale.SetTarget(1.0f);
    }

    bool Update()
    {
        if (ShowTime == -1.0f)
        {
            // return true;
            StartShowPinReward(0);
        }

        BackgroundBouncerScale.Update();

        if (ShowTime == 0.0f)
        {

        }

        ShowTime += 1.0f;
        
        if (ShowTime == 100.0f)
        {
            BackgroundBouncerScale.Set(0.0f);
            ShowTime = -1.0f;
        }

        return false;
    }

    void Render()
    {
        if (ShowTime == -1.0f) return;
        
    }
private:
    CP<RFontFace> OmnesBlack;
    CP<RTexture> BackgroundTexture;
    float ShowTime;
    CBouncer<float> BackgroundBouncerScale;
    v4 Colour;
    CP<RTexture> Icon;
    MMString<wchar_t> Text;
};

CGetPinsTask gPinsTask;
CPinsOverlay* gPinsOverlay;

void UpdatePinOverlay()
{
    // if (gPinsOverlay == NULL)
    //     gPinsOverlay = new CPinsOverlay();
    // gPinsOverlay->Update();
}

void RenderPinOverlay()
{
    // gPinsOverlay->Render();
}

void CGetPinsTask::StartTask()
{
    // AuthenticatedTask.StopTask();
    // AuthenticatedTask.MakeHTTPRequest("http://refresh.jvyden.xyz:2095/lbp/get_my_pins", NULL, E_HTTP_GET, E_HTTP_PRIORITY_HIGH, "application/json", E_HTTP_DIGEST_NONE, false);
    // CHTTPResponseArray* response = new CHTTPResponseArray();
    // *((uint32_t*)response) = 0x0090c598;
    // AuthenticatedTask.StartTask(response, NULL);
}

void CGetPinsTask::Update()
{
    // EHTTPTaskStatus status = AuthenticatedTask.GetTaskStatus();
    // if (status == E_HTTP_TASK_FAIL)
    // {
    //     DebugLogChF(DC_NETWORK, "NET: We failed our get pin request!\n");
    //     AuthenticatedTask.StopTask();
    //     return;
    // }

    // if (status == E_HTTP_TASK_INIT && !Valid)
    // {
    //     DebugLog("STARTING PINS TASK!!!\n");
    //     StartTask();
    //     Valid = true;
    // }
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