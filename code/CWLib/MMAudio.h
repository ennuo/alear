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
class CThing;


namespace CAudio 
{
    AUDIO_HANDLE PlaySample(AUDIO_GROUP& group, char const* name, float param1, v2 const* pos3d, float param2);
    AUDIO_HANDLE PlaySample(AUDIO_GROUP& group, char const* name, CThing* thing, float param1, float param2);
    extern AUDIO_GROUP gSFX;

    extern FMOD::System* System;
    extern FMOD::EventSystem* EventSystem;

};

#endif // MM_AUDIO_H