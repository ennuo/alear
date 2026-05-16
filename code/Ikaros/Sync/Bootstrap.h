#pragma once

#include <thread.h>
#include <CritSec.h>
#include <Sync/Database.h>
#include <Sync/Types.h>

namespace sync
{
    extern int gServerPort;
    extern const char* gServerAddress;

    extern CSyncDatabase Database;
    extern CVector<depot> Depots;
    extern CCriticalSec DepotMutex;

    void LinkDepots();
    void SaveToDepotCache();

    bool Open();
    void Close();
    void Update();
}