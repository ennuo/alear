#pragma once

#include <cell/thread.h>

#define ThreadCreate CreatePPUThread
#define GetCurrentThreadId GetCurrentPPUThreadID
#define GetCurrentThread GetCurrentPPUThread
#define ThreadJoin JoinPPUThread
#define SetThreadPriority SetPPUThreadPriority

bool AmInMainThread();
bool InitThreads();
