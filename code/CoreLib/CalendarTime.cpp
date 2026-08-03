#include <CalendarTime.h>

#include <cell/rtc.h>

bool TryGetCalendarTime(CalendarTime& timestamp)
{
    CellRtcDateTime rtc_time_utc;
    time_t time_out;

    if (cellRtcGetCurrentClockUtc(&rtc_time_utc) != CELL_OK) return false;
    if (cellRtcGetTime_t(&rtc_time_utc, &time_out) != CELL_OK) return false;

    timestamp = time_out;
    return true;
}

u64 GetCalendarMicroseconds()
{
    CellRtcTick tick;
    if (cellRtcGetCurrentTickUtc(&tick) != CELL_OK) return 0;
    return tick.tick;
}

CalendarTime GetCalendarTime()
{
    CalendarTime r;
    TryGetCalendarTime(r);
    return r;
}