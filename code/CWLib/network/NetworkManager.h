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

class CNetworkManager {
public:
    CNetworkMessaging& Messaging;
    CNetworkInputManager& InputManager;
    CNetworkConnectionManager& ConnectionManager;
};

extern CNetworkManager gNetworkManager;

#endif // NETWORK_MANAGER_H