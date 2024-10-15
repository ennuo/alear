#ifndef RESOURCE_GAME_H
#define RESOURCE_GAME_H

class RLevel;

#include "Resource.h"

class RGame : public CResource {
public:
    u32 PauseState;
    float PauseT;
    float PauseAmbientT;
    u32 PauseTrigger;
    u32 SeekState;
    float SeekT;
    float SeekAmbientT;
    u32 SeekTrigger;
    float VCRDamp;
    float FadeTimeLeft;
    CP<RLevel> Level;
private:
    char Pad[0x90];
public:
    bool EditMode;
};

extern RGame* gGame;

#endif // RESOURCE_GAME_H