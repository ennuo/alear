#include "MMAudio.h"
#include "hook.h"

namespace CAudio 
{
    MH_DefineFunc(PlaySample, 0x001aa174, TOC0, AUDIO_HANDLE, AUDIO_GROUP& group, char const* name, CThing* thing, float param1, float param2);
};