#pragma once

#include <thread.h>
#include <CritSec.h>
#include <Sync/Database.h>
#include <Sync/Types.h>
#include <ReadINI.h>

namespace sync
{
    extern CSyncDatabase Database;
    extern CIniSettings Config;
    extern CVector<depot> Depots;
    extern CCriticalSec DepotMutex;

    void LinkDepots();
    void SaveToDepotCache();

    bool Open();
    void Close();
    void Update();
}