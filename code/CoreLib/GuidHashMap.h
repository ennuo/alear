#ifndef GUID_HASH_MAP_H
#define GUID_HASH_MAP_H

#include "CritSec.h"
#include "vector.h"
#include "filepath.h"

#include "GuidHash.h"
#include "SerialiseEnums.h"

class CFileDBRow {
public:
    char* FilePathX;
    CHash FileHash;
    CGUID FileGuid;
    u32 FileSize;
};

class CFileDB {
public:
    virtual ~CFileDB() = 0;
    virtual CFileDBRow* FindByGUID(const CGUID& guid) = 0;
    virtual CFileDBRow* FindByHash(const CHash& hash) = 0;
    virtual CFileDBRow* FindByPath(const CFilePath& path, bool) = 0;
    virtual ReflectReturn Save() = 0;
    virtual void Patch(const CHash& hash, const CGUID& guid, const CFilePath& path) = 0;
    virtual void ValidateFiles() = 0;
public:
    static CFileDB* Construct(CFilePath& path);
public:
    ReflectReturn Load();
public:
    CFilePath Path;
    CVector<CFileDBRow> Files;
    u32 SortedIndex;
};

namespace FileDB
{
    extern CFileDBRow* (*FindByGUID)(const CGUID& guid);
    extern CCriticalSec Mutex;
    extern CVector<CFileDB*> DBs;
}

#endif // GUID_HASH_MAP_H