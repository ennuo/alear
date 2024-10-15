#ifndef MM_AUDIO_H
#define MM_AUDIO_H


#include "thing.h"
#include "Audio/FModAudioHandle.h"

// not importing the fmod headers
namespace FMOD { typedef void* EventGroup; }

template <typename T>
class FModWrapper {
private:
    T* t;
    u32 Frame;
};

typedef CAudio::CFModAudioHandle AUDIO_HANDLE;
typedef FModWrapper<FMOD::EventGroup> AUDIO_GROUP;

namespace CAudio 
{
    extern AUDIO_HANDLE (*PlaySample)(AUDIO_GROUP& group, char const* name, CThing* thing, float param1, float param2);
    extern AUDIO_GROUP g_SFX;
};

#endif // MM_AUDIO_H