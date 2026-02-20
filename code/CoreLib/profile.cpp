#include <profile.h>

CProfileName gProfileRoot("root"); // 9
u32 gCurrentProfileEntry = -1;
u32 gProfileFrameCount; // 11
CProfileEntryMap gProfileEntryMap; // 13
CVector<CProfileEntry> gProfileEntryVector; // 14
u32 CProfileName::Counter; // 15

CProfileName::CProfileName(const char* name) : Name(name), Id(Counter++) // 22
{

}

CProfileEntry::CProfileEntry() : Key(0), // 29
Expanded(), NextSibling(-1), Time(0), AvgTime(0), PeakTime(0),
Hits(0), Name(NULL), Parent(-1), FirstChild(-1)
{

}

void CProfileEntry::Init(CProfileName* name, u32 parentIdx, u32 key) // 43
{
    Parent = parentIdx;
    Name = name;
    Key = key;
    
    if (parentIdx != -1)
    {
        CProfileEntry& entry = gProfileEntryVector[parentIdx];
    
        u32& r = entry.FirstChild;
        while (r != -1)
        {
            entry = gProfileEntryVector[r];
            r = entry.NextSibling;
        }

        r = gCurrentProfileEntry;
    }
}

static bool AmInMainThread() { return true; }

bool CreateProfileEntry(CProfileName* name, u32& last) // 62
{
    if (!AmInMainThread()) return false;

    // if (gCurrentProfileEntry == -1)
    // {
    //     gProfileEntryVector.resize(gProfileEntryVector.size() + 1);
    //     CProfileEntry& entry = gProfileEntryVector.rbegin();
    //     entry.Init(&gProfileRoot, 0, 0);
    // }

    return false;
}

float gProfileSpikeThreshold = 0.2f; // 99