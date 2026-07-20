#pragma once

#include <vector.h>
#include <NpcMoveCommands.h>

class PCreature;

class CJumpTrainer {
public:
    static CVector<CRawVector<SNpcMoveCmd> > JumpList;
public:
    u32 LastList;
    u32 CurrentTrainingIndex;
    u32 CurrentTrainingCmd;
    u32 JumpFrames;
    v2 StartPos;
    CRawVector<v2> Pos;
    CRawVector<SNpcMoveCmd> CommandList;
public:
    static void FillTrainingSet();
public:
    CJumpTrainer();
public:
    void Clear();
    void Solve(SNpcJumpData&);
    void AddFrame(PCreature*);
    void Write();
    void ResetAndTrain();


}