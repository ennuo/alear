#ifndef SYSTEM_H
#define SYSTEM_H

#include "mmtypes.h"

typedef void (InitStepCallback_Close)(void);
typedef bool (InitStepCallback_Init)(void);

class CInitStep {
public:
    char* DebugText;
    bool* Check_This_Bool_Before_Init;
    InitStepCallback_Init* InitFunc;
    InitStepCallback_Close* CloseFunc;
    InitStepCallback_Init* PostResourceInitFunc;
    bool Inited;
    CInitStep* ChainTo;
};

void AddInitSteps(CInitStep* newsteps);
extern CInitStep g_InitSteps[];
extern CInitStep g_Ps3Test1InitSteps[];

#endif // SYSTEM_H