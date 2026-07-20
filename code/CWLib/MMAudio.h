#ifndef MM_AUDIO_H
#define MM_AUDIO_H


#include <fmod.hpp>
#include <fmod_event.h>

#include "Audio/FModAudioHandle.h"
#include "Audio/FMODFile.h"

template <typename T>
class FModWrapper {
public:
    T* t;
    u32 Frame;
};

typedef CAudio::CFModAudioHandle AUDIO_HANDLE;
typedef FModWrapper<FMOD::EventGroup> AUDIO_GROUP;
typedef FMOD_RESULT AUDIO_RESULT;
class CThing;


namespace CAudio 
{
    const float DEFAULT_PARAM = -10000.0f;

    AUDIO_HANDLE PlaySample(AUDIO_GROUP& group, char const* name, float param1 = DEFAULT_PARAM, v2 const* pos3d = NULL, float param2 = DEFAULT_PARAM);
    AUDIO_HANDLE PlaySample(AUDIO_GROUP& group, char const* name, CThing* thing, float param1 = DEFAULT_PARAM, float param2 = DEFAULT_PARAM);
    AUDIO_RESULT StopSample(AUDIO_HANDLE& h);
    CGUID GetCurrMusicGuid();

    extern AUDIO_GROUP gSFX;
    extern AUDIO_GROUP gLBP2SFX;

    extern FMOD::System* System;
    extern FMOD::EventSystem* EventSystem;

};

#endif // MM_AUDIO_H