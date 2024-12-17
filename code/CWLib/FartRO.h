#ifndef FART_READ_ONLY_H
#define FART_READ_ONLY_H

#include "Fart.h"

const u32 FARC = 0x46415243;

struct Footer {
    u32 count;
    u32 magic;
};

enum EFartROState {
    EFartROState_Disabled,
    EFartROState_Enabled,
    EFartROState_Updating
};

class CFartRO : public CCache { // FartRO.h: 50
public:
    class CFAT {
    public:
        bool operator<(CFAT const& rhs) const { return hash.Compare(rhs.hash) < 0; }
    public:
        CHash hash;
        u32 offset;
        u32 size;
    };

    enum ETryGetReaderResult {
        ETryGetReaderResult_False,
        ETryGetReaderResult_True,
        ETryGetReaderResult_Wait
    };
public:
    void CloseCache(bool updating);
    bool OpenCache();
public:
    CFilePath fp;
    CRawVector<CFAT> FAT;
    FileHandle fd;
    // FileHandle fd_read_cache; deploy only?
    bool Secure;
    bool Slow;
    bool WantCheckCache;
    u32 Readers;
    EFartROState State;
};

class CFartManyRO : public CCache {
public:
    class CFAT : public CFartRO::CFAT {
    public:
        CFartRO* Owner;
    };
public:
    CVector<CFartRO*> Farts;
    CRawVector<CFAT> FAT;
};

extern CFartRO* (*MakeROCache)(CFilePath& farc_file, bool enabled, bool secure);
extern bool (*IncorporateFartRO)(CFartRO* cache);

#endif // FART_READ_ONLY_H