#include "MMAudio.h"
#include "hook.h"

namespace CAudio 
{
    MH_DefineFunc(PlaySample_Internal, 0x001aa174, TOC0, AUDIO_HANDLE, AUDIO_GROUP&, char const*, CThing*, float, float);
    AUDIO_HANDLE PlaySample(AUDIO_GROUP& group, char const* name, CThing* thing, float param1, float param2)
    {
        return PlaySample_Internal(group, name, thing, param1, param2);
    }

    MH_DefineFunc(PlaySample_3D_Internal, 0x001a54e0, TOC0, AUDIO_HANDLE, AUDIO_GROUP&, char const*, float, v2 const*, float);
    AUDIO_HANDLE PlaySample(AUDIO_GROUP& group, char const* name, float param1, v2 const* pos3d, float param2)
    {
        return PlaySample_3D_Internal(group, name, param1, pos3d, param2);
    }

    MH_DefineFunc(StopSample_Internal, 0x001a30cc, TOC0, AUDIO_RESULT, AUDIO_HANDLE&);
    AUDIO_RESULT StopSample(AUDIO_HANDLE& h)
    {
        return StopSample_Internal(h);
    }

    MH_DefineFunc(GetCurrMusicGuid_Internal, 0x0019bdf8, TOC0, CGUID);
    CGUID GetCurrMusicGuid()
    {
        return GetCurrMusicGuid_Internal();
    }
};