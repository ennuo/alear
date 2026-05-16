#include <Sync/Database.h>
#include <Sync/Bootstrap.h>

namespace sync
{
    CFileDBRow* CSyncDatabase::FindByGUID(const CGUID& guid)
    {
        CCSLock lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            const depot& depot = Depots[i];
            if (depot.Database == NULL) continue;
            CFileDBRow* row = depot.Database->FindByGUID(guid);
            if (row != NULL)
                return row;
        }

        return NULL;
    }

    CFileDBRow* CSyncDatabase::FindByHash(const CHash& hash)
    {
        CCSLock lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            const depot& depot = Depots[i];
            if (depot.Database == NULL) continue;
            CFileDBRow* row = depot.Database->FindByHash(hash);
            if (row != NULL)
                return row;
        }

        return NULL;
    }

    CFileDBRow* CSyncDatabase::FindByPath(const CFilePath& path, bool create)
    {
        CCSLock lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            const depot& depot = Depots[i];
            if (depot.Database == NULL) continue;
            CFileDBRow* row = depot.Database->FindByPath(path, create);
            if (row != NULL)
                return row;
        }

        return NULL;
    }
}