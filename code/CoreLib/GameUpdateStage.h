#ifndef GAME_UPDATE_STAGE_H
#define GAME_UPDATE_STAGE_H

enum EMainGameUpdateStage
{
    E_UPDATE_STAGE_SYNCED,
    E_UPDATE_STAGE_PREDICTED_OR_RENDER,
    E_UPDATE_STAGE_OTHER_WORLD,
    E_UPDATE_STAGE_LOADING,
    E_UPDATE_STAGE_COUNT
};

extern EMainGameUpdateStage gMainGameUpdateStage;

class CMainGameStageOverride {
public:
    CMainGameStageOverride(EMainGameUpdateStage new_stage);
    ~CMainGameStageOverride();

private:
    EMainGameUpdateStage PrevMainGameUpdateStage;
};

#endif // GAME_UPDATE_STAGE_H