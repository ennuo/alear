#include "System.h"

void AddInitSteps(CInitStep* newsteps)
{
	CInitStep* step = g_InitSteps;
	CInitStep* last = NULL;
	while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
	{
		step = ((last = step)->ChainTo == NULL) ? ++step : last->ChainTo;
	}
	last->ChainTo = newsteps;
}