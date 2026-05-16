#pragma once

#include <GuidHashMap.h>
#include <Sync/Types.h>

namespace sync
{
    class CSyncDatabase : public CFileDB {
    public:
        inline CSyncDatabase() : CFileDB("") {}
    public:
        CFileDBRow* FindByGUID(const CGUID& guid);
        CFileDBRow* FindByHash(const CHash& hash);
        CFileDBRow* FindByPath(const CFilePath& path, bool create = false);
    };
}