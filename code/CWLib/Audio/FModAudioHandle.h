#ifndef AUDIO_FMOD_AUDIO_HANDLE_H
#define AUDIO_FMOD_AUDIO_HANDLE_H

#include "vector.h"

namespace CAudio 
{
    class CFModEventInstanceTracker {
    public:
        void* /* Event* */ FModEvent;
        u32 RefCount;
        u32 Next;
        u32 Prev;
        u32 FModEventTrackerIndex;
    };

    extern CVector<CFModEventInstanceTracker> gFModEventInstanceList;

    class CFModAudioHandle {
    public:
        inline CFModAudioHandle()
        {
            FModEventInstanceTrackerIndex = 0;
            Frame = 0;
        }

        inline ~CFModAudioHandle()
        {
            if (FModEventInstanceTrackerIndex != 0)
                gFModEventInstanceList[FModEventInstanceTrackerIndex - 1].RefCount -= 1;
        }
    private:
        u32 FModEventInstanceTrackerIndex;
        u32 Frame;
    };
};

#endif // AUDIO_FMOD_AUDIO_HANDLE_H