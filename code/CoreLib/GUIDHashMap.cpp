#include "GuidHashMap.h"
#include "hook.h"
#include "mem_allocator.h"

#include <algorithm>
#include <functional>

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

void CFileDBRow::SetPath(const char* path)
{
    if (path != NULL)
    {
        // technically uses a pool womp
        int len = StringLength(path) + 1;
        char* fp = new char[len];
        strcpy(fp, path);
        FilePathX = fp;

        return;
    }

    FilePathX = NULL;
    return;
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
    if (StringCompareN(path, gBaseDir.c_str(), gBaseDir.Length()))
        path += gBaseDir.Length();

    for (CFileDBRow* row = Files.begin(); row != Files.end(); ++row)
    {
        if (StringCompare(row->GetPath(), path) == 0)
            return row;
    }

    return NULL;
}


CFileDB* CFileDB::Construct(CFilePath& path)
{
    CFileDB* database = (CFileDB*)CAllocatorMM::Malloc(gOtherBucket, sizeof(CFileDB));
    memset(database, 0, sizeof(CFileDB));
    *(u32*)database = 0x0090c530;
    database->Path.Assign(path.c_str());
    return database;
}

namespace FileDB {
    MH_DefineFunc(FindByGUID, 0x0057c8d8, TOC1, CFileDBRow*, const CGUID& guid);
    MH_DefineFunc(FindByPath, 0x0057cb90, TOC1, CFileDBRow*, const CFilePath&, bool);
};


MH_DefineFunc(CFileDB_Load, 0x0057d13c, TOC1, ReflectReturn, CFileDB*);
ReflectReturn CFileDB::Load()
{
    return CFileDB_Load(this);
}