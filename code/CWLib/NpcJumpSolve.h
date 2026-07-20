#pragma once

#include <NpcMoveCommands.h>
#include <NpcJumpSolve.h>

#include <vector.h>

struct SNpcJumpData {
    v2 Min;
    v2 Max;
    v2 Apex;
    float A;
    float B;
    float C;
    bool Flipped;
    CRawVector<SNpcMoveCmd> CommandList;

    SNpcJumpData();
    bool GetX(float, float&) const;
    float GetY(float) const;
    void Clear();
};

class CNpcJumpSolver {
private:
    v2 CurSource; // 0x0
    v2 CurTarget0; // 0x10
    v2 CurTarget1; // 0x20

    int CurrentJump; // 0x60
    u32 CurrentJumpPos; // 0x64
    // 0x68
    bool IsCurrentJumpFlipped; // 0x6c

    // CVector<SNpcJumpData> JumpData;
    // SNpcJumpData Best;
    // v2 CurSource;
    // v2 CurTarget;
    // float OvershootAllowed;
    // u32 NumCorrections;
    // float MaxEffectiveJumpHeight;
    // bool Trained;
    // CJumpTrainer Trainer;
    // v2 LastWorldPos;
};