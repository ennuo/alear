#include "GuidHashMap.h"
#include "hook.h"
#include "mem_allocator.h"

#include <algorithm>
#include <functional>
#include <Serialise.h>
#include <mmalex.h>

CFileDBRow::CFileDBRow()
{
    memset(this, 0, sizeof(CFileDBRow));
}

CFileDBRow::~CFileDBRow()
{

}

void CFileDBRow::Init(CGUID guid, const char* path)
{
    FileGuid = guid;
    FileSize = 0;
    SetPath(path);
}

void CFileDBRow::Update(const CHash& hash, u32 size)
{
    FileHash = hash;
    FileSize = size;
}

const char* CFileDBRow::GetPath() const
{
    return FilePathX ? FilePathX : "";
}

const char* CFileDBRow::GetFilename() const
{
    if (!FilePathX) return "";
    const char* s = strrchr(FilePathX, '/');
    return s ? s + 1 : FilePathX;
}

// This function assumes the path string
// was allocated by the database that owns this row.
void CFileDBRow::SetPath(const char* path)
{
    FilePathX = path;
    LocalGuid = 0x80000000ul | mmalex::crc32(0, FilePathX, StringLength(FilePathX));
}

CFileDB::~CFileDB()
{

}

CFileDBRow* CFileDB::FindByGUID(const CGUID& guid)
{
    if (!guid || Files.size() == 0) return NULL;

    CFileDBRow* sorted_index = Files.begin() + SortedIndex;
    CFileDBRow* row = std::lower_bound(Files.begin(), sorted_index, guid, SCompareGUID());
    if (row != sorted_index && row->GetGUID() == guid)
        return row;

    for (row = sorted_index; row != Files.end(); ++row)
    {
        if (row->GetGUID() == guid)
            return row;
    }

    return NULL;
}

CFileDBRow* CFileDB::FindByHash(const CHash& hash)
{
    if (!hash) return NULL;

    for (CFileDBRow* row = Files.begin(); row != Files.end(); ++row)
    {
        if (hash == row->GetHash())
            return row;
    }

    return NULL;
}

CFileDBRow* CFileDB::FindByPath(const CFilePath& fp, bool create)
{
    if (!fp.IsValid()) return NULL;

    const char* path = fp.c_str();
    if (StringCompareN(path, gBaseDir.c_str(), gBaseDir.Length()) == 0)
        path += gBaseDir.Length();

    for (CFileDBRow* row = Files.begin(); row != Files.end(); ++row)
    {
        if (StringCompare(row->GetPath(), path) == 0)
            return row;
    }

    return NULL;
}

namespace FileDB {
    MH_DefineFunc(FindByGUID, 0x0057c8d8, TOC1, CFileDBRow*, const CGUID& guid);
    MH_DefineFunc(FindByPath, 0x0057cb90, TOC1, CFileDBRow*, const CFilePath&, bool);
};

class CDatabaseLoadVector : public CReflectionLoadVector {
public:
    CDatabaseLoadVector(CFileDB* database, CBaseVector<char>* vec) : CReflectionLoadVector(vec)
    {
        Database = database;
        SetCompressionFlags(0);
    }
public:
    CFileDB* Database;
};

template <typename R>
ReflectReturn Reflect(R& r, CFileDBRow& d)
{
    ReflectReturn ret;

    char* path = (char*)d.FilePathX;
    u32 length = path != NULL ? StringLength(path) : 0;
    if ((ret = Reflect(r, length)) != REFLECT_OK) return ret;

    if (r.GetLoading())
    {
        path = (char*)r.Database->StringAllocator.alloc(length + 1);
        path[length] = '\0';
    }

    if ((ret = r.ReadWrite(path, length)) != REFLECT_OK) return ret;

    CHash hash = d.GetHash();
    CGUID guid = d.GetGUID();
    int size = d.GetSize();
    u64 timestamp = 0;

    if ((ret = Reflect(r, timestamp)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, size)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, hash)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, guid.guid)) != REFLECT_OK) return ret;
    
    if (r.GetLoading())
    {
        d.Init(guid, path);
        d.Update(hash, size);
    }

    return REFLECT_OK;
}


ReflectReturn CFileDB::Load()
{
    Files.clear();
    SortedIndex = 0;
    StringAllocator.~CMemPoolSimple();
    new (&StringAllocator) CMemPoolSimple(0x4000);

    ByteArray vec;

    CDatabaseLoadVector r(this, &vec);
    if (!FileLoad(Path, vec, NULL)) return REFLECT_FILEIO_FAILURE;

    ReflectReturn ret;
    u32 revision;
    if ((ret = Reflect(r, revision)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, Files)) != REFLECT_OK) return ret;

    // Not sure why they don't just sort it instead
    // of using the sorted index, but whatever.
    std::sort(Files.begin(), Files.end(), SCompareGUID());
    SortedIndex = Files.size();

    return REFLECT_OK;
}

bool FileDB::RemapLocalGUID(const CGUID& in, CGUID& out)
{
    if (in < 0)
    {
        const CFileDBRow* row = FindByGUID(in);
        if (row != NULL) return false;

        CCSLock _lock(&FileDB::Mutex, __FILE__, __LINE__);
        for (u32 i = 0; i < FileDB::DBs.size(); ++i)
        {
            CFileDB* database = FileDB::DBs[i];
            for (u32 j = 0; j < database->Files.size(); ++j)
            {
                CFileDBRow& file = database->Files[j];
                if (CGUID(file.LocalGuid) == in)
                {
                    out = file.FileGuid;
                    return true;
                }
            }   
        }
    }

    return false;
}