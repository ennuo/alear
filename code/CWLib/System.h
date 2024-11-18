#ifndef SYSTEM_H
#define SYSTEM_H



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
extern CInitStep gInitSteps[];
extern CInitStep gPs3Test1InitSteps[];

extern bool(*WantQuitOrWantQuitRequested)();

#endif // SYSTEM_H