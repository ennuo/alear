#include "GuidHashMap.h"
#include "hook.h"
#include "mem_allocator.h"

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
};


MH_DefineFunc(CFileDB_Load, 0x0057d13c, TOC1, ReflectReturn, CFileDB*);
ReflectReturn CFileDB::Load()
{
    return CFileDB_Load(this);
}