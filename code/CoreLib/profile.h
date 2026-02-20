#pragma once

#include <utility>

#include <Clock.h>
#include <vector.h>

typedef CVector<std::pair<u32, u32> > CProfileEntryMap;

class CProfileName {
public:
    static u32 Counter;
public:
    CProfileName(const char* name);
public:
    const char* Name;
    u32 Id;
};

class CProfileEntry { // 59
public:
    CProfileEntry();
    void Init(CProfileName*, u32, u32);
public:
    u64 Time;
    u64 AvgTime;
    u64 PeakTime;
    u64 Hits;
    CProfileName* Name;
    u32 Key;
    u32 Parent;
    u32 FirstChild;
    u32 NextSibling;
    bool Expanded;
};

bool CreateProfileEntry(CProfileName* name, u32& last);

class CProfileScope { // 80
public:
    CProfileScope(CProfileName* name)
    {
        Last = -1;
        if (CreateProfileEntry(name, Last))
            StartTime = GetClock();
    }

    ~CProfileScope();
public:
    u64 StartTime;
    u32 Last;
};
