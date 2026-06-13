#ifndef GUID_HASH_MAP_H
#define GUID_HASH_MAP_H

#include "CritSec.h"
#include "vector.h"
#include "filepath.h"

#include "GuidHash.h"
#include "SerialiseEnums.h"
#include <mem_pool_simple.h>

class CFileDBRow {
public:
    CFileDBRow();
    ~CFileDBRow();
public:
    const char* GetPath() const;
    const char* GetFilename() const;
    inline const CHash& GetHash() const { return FileHash; }
    inline const CGUID& GetGUID() const { return FileGuid; }
    inline u32 GetSize() const { return FileSize; }
    void SetPath(const char* path);
    void Update(const CHash& hash, u32 size);
    void Init(CGUID guid, const char* path);
public:
    const char* FilePathX;
    CHash FileHash;
    CGUID FileGuid;
    u32 FileSize;
};

typedef CVector<CFileDBRow> V_CFileDBRow;

struct SComparepath
{
    inline bool operator()(const CFileDBRow& lhs, const CFileDBRow& rhs)
    {
        return StringCompare(lhs.GetPath(), rhs.GetPath());
    }

    inline bool operator()(const CFileDBRow& lhs, const CFilePath& rhs)
    {
        return StringCompare(lhs.GetPath(), rhs.c_str());
    }

    inline bool operator()(const CFilePath& lhs, const CFileDBRow& rhs)
    {
        return StringCompare(lhs.c_str(), rhs.GetPath());
    }
};

struct SCompareGUID
{
    inline bool operator()(const CFileDBRow& lhs, const CFileDBRow& rhs)
    {
        return lhs.GetGUID() < rhs.GetGUID();
    }

    inline bool operator()(const CGUID& lhs, const CFileDBRow& rhs)
    {
        return lhs < rhs.GetGUID();
    }

    inline bool operator()(const CFileDBRow& lhs, const CGUID& rhs)
    {
        return lhs.GetGUID() < rhs;
    }
};

class CFileDB {
public:
    inline CFileDB(const CFilePath& fp) : Path(fp), Files(), SortedIndex(), StringAllocator(0x4000)
    {
        
    }
public:
    virtual ~CFileDB();
    virtual CFileDBRow* FindByGUID(const CGUID& guid);
    virtual CFileDBRow* FindByHash(const CHash& hash);
    virtual CFileDBRow* FindByPath(const CFilePath& path, bool create = false);
    inline virtual ReflectReturn Save() { return REFLECT_OK; }
    inline virtual void Patch(const CHash& hash, const CGUID& guid, const CFilePath& path) {}
    inline virtual void ValidateFiles() {}
public:
    ReflectReturn Load();
public:
    CFilePath Path;
    V_CFileDBRow Files;
    u32 SortedIndex;
    CMemPoolSimple StringAllocator;
};

typedef CVector<CFileDB*> V_CFileDB;

namespace FileDB
{
    extern CFileDBRow* (*FindByGUID)(const CGUID& guid);
    extern CFileDBRow* (*FindByPath)(const CFilePath& fp, bool create);
    extern CCriticalSec Mutex;
    extern V_CFileDB DBs;
}

#endif // GUID_HASH_MAP_H