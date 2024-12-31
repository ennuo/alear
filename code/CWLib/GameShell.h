#ifndef GAME_SHELL_H
#define GAME_SHELL_H

#include "OverlayUI.h"

class CGameShell {
public:
    COverlayUI* OverlayUI;
};

extern CGameShell* gGameShell;

#endif // GAME_SHELL_H