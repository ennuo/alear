#pragma once

#include <thread.h>
#include <CritSec.h>
#include <Sync/Database.h>
#include <Sync/Types.h>
#include <ReadINI.h>
#include <TextRange.h>

namespace sync
{
    extern CSyncDatabase Database;
    extern CIniSettings Config;
    extern CVector<depot> Depots;
    extern CCriticalSec DepotMutex;
    extern CVector<TextRange<char> > DefinedBranches;


    void LinkDepots();
    void SaveToDepotCache();
    bool IsBranchDefined(const TextRange<char>& branch);

    bool Open();
    void Close();
    void Update();
}