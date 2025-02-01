#include "GameUpdateStage.h"

CMainGameStageOverride::CMainGameStageOverride(EMainGameUpdateStage new_stage)
{
    PrevMainGameUpdateStage = gMainGameUpdateStage;
    gMainGameUpdateStage = new_stage;
}

CMainGameStageOverride::~CMainGameStageOverride()
{
    gMainGameUpdateStage = PrevMainGameUpdateStage;
}