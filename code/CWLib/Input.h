#pragma once

#include <PadEnums.h>
#include <input.inl>
#include <InputMode.h>
#include <ButtonPrompts.inl>
#include <PlayerNumber.inl>
#include <StickEnum.h>


class CInput {
private:
    u32 GetListenerType(EButtonConfig button_config) const;
    void SetButtonPrompt(EButtonConfig button_config, wchar_t const* lams_key = (const wchar_t*)NULL) const;
    bool IsClicked(EButtonConfig button_config, u32 buttons) const;
public:
    v4 GetStick(u32 listener_type, EStick stick) const;
public:
    inline bool IsJustClicked(EButtonConfig n, wchar_t const* button_prompt = (const wchar_t*)NULL) const
    {
        if (GetListenerType(n) & InputMode)
            SetButtonPrompt(n, button_prompt);
        return IsClicked(n, Buttons) && !IsClicked(n, ButtonsOld) && (GetListenerType(n) & InputModeOld) != 0;
    }

    inline bool IsClicked(EButtonConfig n, wchar_t const* button_prompt = (const wchar_t*)NULL) const
    {
        if (GetListenerType(n) & InputMode)
            SetButtonPrompt(n, button_prompt);
        return IsClicked(n, Buttons);
    }

    inline u16 GetPressureData(u32 listener_type, EButtonPressure pressure) const
    {
        return listener_type & InputMode ? PressureData[pressure] : 0;
    }

public:
    u32 InputMode; // 0x0
    u32 RumbleFrames; // 0x4
    u32 ContinuosRumbleFrames; // 0x8
    u32 InputModeOld; // 0xc
    bool Alive; // 0x10
    v4 LeftStick; // 0x20
    v4 RightStick; // 0x30
    u32 Buttons; // 0x40
    u32 ButtonsOld; // 0x44
    u16 SensorData[4]; // 0x48
    float SensorDataSmooth[4]; // 0x50
    v4 SensorDir; // 0x60
    v4 SensorDirOld; // 0x70
    u16 PressureData[12]; // 0x80
    u32 stick_move_frame[2];
    EPlayerNumber PlayerNumber;
    bool ControllingPauseMenu;
    bool InSaveDataOperation;
    bool InAttractionLoop;
    bool Activity;
};

extern void (*_SetButtonPrompt)(EButtonPrompts button, wchar_t const* lams_key, EPlayerNumber player);