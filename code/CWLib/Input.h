#ifndef INPUT_H
#define INPUT_H

#include "mmtypes.h"
#include "PadEnums.h"

class CInput {
public:
    u32 InputMode;
    u32 RumbleFrames;
    u32 ContinuosRumbleFrames;
    u32 InputModeOld;
    bool Alive;
    v2 LeftStick;
    v2 RightStick;
    u32 Buttons;
    u32 ButtonsOld;
};

#endif // INPUT_H