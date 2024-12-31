#ifndef INPUT_H
#define INPUT_H

#include "PadEnums.h"
#include "input.inl"
#include "InputMode.h"
#include "ButtonPrompts.inl"
#include "PlayerNumber.inl"

class CInput {
private:
    u32 GetListenerType(EButtonConfig button_config) const;
    void SetButtonPrompt(EButtonConfig button_config, wchar_t const* lams_key) const;
    bool IsClicked(EButtonConfig button_config, u32 buttons) const;
public:
    inline bool IsJustClicked(EButtonConfig n, wchar_t const* button_prompt) const
    {
        if (GetListenerType(n) & InputMode)
            SetButtonPrompt(n, button_prompt);
        return IsClicked(n, Buttons) && !IsClicked(n, ButtonsOld) && (GetListenerType(n) & InputModeOld) != 0;
    }

    inline bool IsClicked(EButtonConfig n, wchar_t const* button_prompt) const
    {
        if (GetListenerType(n) & InputMode)
            SetButtonPrompt(n, button_prompt);
        return IsClicked(n, Buttons);
    }

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
    u16 SensorData[4];
    float SensorDataSmooth[4];
    v4 SensorDir;
    v4 SensorDirOld;
    u16 PressureData[12];
    u32 stick_move_frame[2];
    EPlayerNumber PlayerNumber;
    bool ControllingPauseMenu;
    bool InSaveDataOperation;
    bool InAttractionLoop;
    bool Activity;
};

#endif // INPUT_H