#pragma once

struct SNpcMoveCmd {
    u32 Buttons;
    u8 StickX;
    u8 StickY;

    SNpcMoveCmd();
    SNpcMoveCmd(v2, u32);

    v2 GetLeftStick() const;
    u32 GetButtons() const;
    void AddButtons(u32);
    void SetLeftStick(v2);
    void Clear();
};