#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "PlayerNumber.inl"

class CNetworkMessaging {
private:
};

class CNetworkInputManager {
public:
    EPlayerNumber GetLocalLeadersPlayerNumber();
};

class CNetworkConnectionManager {
private:
    char Pad[0x20130];
public:
    bool DownloadOK;
};

class CUserSelectorForPad {
public:
    inline bool IsSelectionScreenCreated() const { return SelectionScreenCreated; }
private:
    bool Active;
    bool SelectionScreenCreated;
    bool Finished;
};

class CNetworkGameDataManager {
public:
    CUserSelectorForPad UserSelectorForPad;
};

class CNetworkManager {
public:
    CNetworkMessaging& Messaging;
    CNetworkInputManager& InputManager;
    CNetworkConnectionManager& ConnectionManager;
private:
    void* Managers[2];
public:
    CNetworkGameDataManager& GameDataManager;
};

extern CNetworkManager gNetworkManager;

#endif // NETWORK_MANAGER_H