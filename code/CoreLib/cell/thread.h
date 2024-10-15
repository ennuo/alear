#ifndef THREAD_H
#define THREAD_H

#include <sys/ppu_thread.h>

typedef sys_ppu_thread_t THREAD;
typedef sys_ppu_thread_t THREADID;
typedef void (*THREADPROC)(uint64_t);

THREAD CreatePPUThread(THREADPROC threadproc, uint64_t thread_arg, const char* name, int priority, int stacksize, bool joinable);
THREAD GetCurrentPPUThread();
THREADID GetCurrentPPUThreadID();
bool SetPPUThreadPriority(THREAD thread, int priority);
bool JoinPPUThread(THREAD t, uint64_t* thread_retval);
void ExitPPUThread(uint64_t retval);

void ThreadSleep(int ms);

#endif // THREAD_H