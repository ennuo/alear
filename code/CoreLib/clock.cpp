#include <Clock.h>

#include <sys/time_util.h>
#include <sys/sys_time.h>

extern u64 gStartTime;
u64 gClockFreq;
float gClockFreqInv;

void InitClock()
{
    gClockFreq = sys_time_get_timebase_frequency();
    gClockFreqInv = 1.0f / (float)gClockFreq;
}

u64 GetClock()
{
    u64 clock;
    SYS_TIMEBASE_GET(clock);
    if (gStartTime == 0)
        gStartTime = clock;
    return clock - gStartTime;
}

u64 GetClockFreq()
{
    if (gClockFreq == 0) InitClock();
    return gClockFreq;
}

float GetClockFreqInv()
{
    if (gClockFreq == 0) InitClock();
    return gClockFreqInv;
}

bool InitPerformanceTimers()
{
    if (gClockFreq == 0) InitClock();
    return true;
}

float ToSeconds(u64 clocktime)
{
    return (float)clocktime * GetClockFreqInv();
}

float ToMilliseconds(u64 clocktime)
{
    return (float)clocktime * 1000.0f * GetClockFreqInv();
}

float GetClockSeconds()
{
    return (float)GetClock() * GetClockFreqInv();
}

float GetClockMilliSeconds()
{
    return (float)GetClock() * 1000.0f * GetClockFreqInv();
}

u64 GetClockMilliSecondsInt()
{
    if (gClockFreq == 0) InitClock();
    return GetClock() * 1000 / gClockFreq;
}
