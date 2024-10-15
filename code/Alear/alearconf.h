#ifndef ALEAR_CONFIG_H
#define ALEAR_CONFIG_H

#include "refcount.h"
#include "ResourceTranslationTable.h"

extern bool gUseDivergenceCheck;
extern bool gUsePopitGradients;
extern bool gHideMSPF;
extern float gFarDist;

extern StaticCP<RTranslationTable> gAlearTrans;

void AlearInitConf();

#endif // ALEAR_CONFIG_H