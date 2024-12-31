#ifndef AUDIO_FMOD_FILE_H
#define AUDIO_FMOD_FILE_H

#include <refcount.h>
#include <ResourceFileOfBytes.h>
#include <GuidHash.h>

namespace CAudio
{
    class FMODFile {
    public:
        CP<RFilename> Handle;
        CGUID Key;
    };
    
    extern FMODFile* gFMODFiles;
    extern int gFMODFileSize;
}

#endif // AUDIO_FMOD_FILE_H