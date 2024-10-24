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

class CNetworkManager {
public:
    CNetworkMessaging& Messaging;
    CNetworkInputManager& InputManager;
};

extern CNetworkManager gNetworkManager;

#endif // NETWORK_MANAGER_H