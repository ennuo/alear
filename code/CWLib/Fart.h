#ifndef FART_H
#define FART_H

#include <vector.h>
#include <sha1.h>
#include <CritSec.h>
#include <GuidHash.h>
#include <filepath.h>

enum CacheType {
    CT_READONLY,
    CT_ACCEL,
    CT_TEMP,
    CT_DOWNLOAD,
    CT_FSB,
    CT_LEVEL,
    CT_LITTLEFARTBUILD,
    CT_SAVEGAME_FIRST,
    CT_SAVEGAME_LAST = 39,
    CT_COUNT
};

struct SResourceReader;

class CCache { // fart.h: 18
public:
    virtual ~CCache() = 0;
    virtual bool IsSlow(const SResourceReader&) = 0;
    virtual bool GetReader(const CHash&, SResourceReader&) = 0;
    virtual void CloseReader(SResourceReader&, bool) = 0;
    virtual bool Unlink(const CHash&) = 0;
    virtual bool GetSize(const CHash&, u32&) = 0;
    virtual bool Put(CHash&, const void*, u32) = 0;
protected:
    CCriticalSec Mutex;
};

struct SResourceReader { // fart.h: 37
    CCache* Owner;
    FileHandle Handle;
    u32 Offset;
    u32 Size;
    s32 BytesRead;
    u32 OwnerData;
    CSHA1Context RollingHash;
    CHash OriginalHash;
};

class CQueef { // fart.h: 61
enum { buffer_size = 32768 };
private:
    u8 Buffer[buffer_size];
    SResourceReader Reader;
    bool Used;
    FileHandle Writer;
    u32 Size;
};

extern bool (*CloseCaches)();
extern bool (*InitCaches)();
extern CCache* gCaches[CT_COUNT];

#endif // FART_H