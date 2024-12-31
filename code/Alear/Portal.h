#ifndef PORTAL_H
#define PORTAL_H

#include <MemoryTexture.h>

class CPortalManager {
public:
    static CMemoryTexture s_PortalTexture;
public:
    CPortalManager();
};

extern CPortalManager g_PortalManager;
void AlearInitPortalHook();

#endif // PORTAL_H