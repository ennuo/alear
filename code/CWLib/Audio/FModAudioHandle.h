#ifndef AUDIO_FMOD_AUDIO_HANDLE_H
#define AUDIO_FMOD_AUDIO_HANDLE_H



namespace CAudio 
{
    class CFModAudioHandle {
    private:
        u32 FModEventInstanceTrackerIndex;
        u32 Frame;
    };
};

#endif // AUDIO_FMOD_AUDIO_HANDLE_H