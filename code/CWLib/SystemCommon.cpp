#include "System.h"
#include <hook.h>

MH_DefineFunc(WantQuitOrWantQuitRequested, 0x0001dea8, TOC0, bool);

void AddInitSteps(CInitStep* newsteps)
{
	CInitStep* step = gInitSteps;
	CInitStep* last = NULL;
	while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
	{
		step = ((last = step)->ChainTo == NULL) ? ++step : last->ChainTo;
	}
	last->ChainTo = newsteps;
}