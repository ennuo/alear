#ifndef RNP_MANAGER_H
#define RNP_MANAGER_H

#include "network/NetworkUtilsNP.h"

class CPlayerErrorData { // 37
public:
    NetworkPlayerID PlayerID;
    float LastBusyTime;
    float LastTimeoutTime;
public:
    inline CPlayerErrorData() : PlayerID(), LastBusyTime(), LastTimeoutTime() {}
    inline CPlayerErrorData(const NetworkPlayerID& player_id) : PlayerID(player_id), LastBusyTime(), LastTimeoutTime() {}
    inline bool IsValid() { return true; }
};

class CRNPManager {
enum EPrepareStage {
    E_PREPARE_STAGE_INIT,
    E_PREPARE_STAGE_FULLY_CONNECTED
};
public:
    EPrepareStage PrepareStage;
    CPlayerErrorData PlayerErrorData[4];
    CRNPLoadingTask LoadingTasks[8];
    CRNPSendingTask SendingTasks[16];
    float TimeToSendNextBlock;
    u32 NextSendTaskIndex;
    CRandomStream RandomNumbers;
    u64 LastUpdateTime;
    RNPFileDB* RNPDB;
};

#endif // RNP_MANAGER_H