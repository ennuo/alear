#include <System.h>
#include <Clock.h>
#include <thread.h>
#include <DebugLog.h>

extern bool gWantQuitRequested;
extern bool gWantQuit;
extern u64 gSetWantQuitTime;
extern u64 gCheckWantQuitTime;
extern u64 gCheckWantQuitOrWantQuitRequestedTime;

bool WantQuit()
{
	if (AmInMainThread())
		gCheckWantQuitTime = GetClock();
	return gWantQuit;
}

bool WantQuitOrWantQuitRequested()
{
	if (AmInMainThread())
		gCheckWantQuitOrWantQuitRequestedTime = GetClock();
	return gWantQuit || gWantQuitRequested;
}

void SetWantQuit(bool wantQuit)
{
	if (wantQuit && !gWantQuitRequested)
	{
		gSetWantQuitTime = GetClock();
		MMLogCh(DC_INIT, "SetWantQuit() at %.2f %.2f\n", ToSeconds(gSetWantQuitTime), GetClockSeconds());
	}

	gWantQuitRequested = wantQuit;
}

u64 GetQuitTime()
{
	return gSetWantQuitTime;
}


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