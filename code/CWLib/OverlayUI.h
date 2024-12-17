#ifndef OVERLAY_UI_H
#define OVERLAY_UI_H

#include <refcount.h>
#include <vector.h>

#include "vm/VMTypes.h"
#include "thing.h"
#include "Typeinator.h"
#include "PadIndex.inl"
#include "PlanetEventHandler.h"

#include "ResourceLevel.h"
#include "ResourceGFXTexture.h"

#include "gooey/GooeyNodeManager.h"

struct SScriptedOverlay {
    CThingPtr Thing;
    CSignature Signature;
};

enum EStartMenuEntryPoint {
    SMEP_PAUSE_MENU,
    SMEP_POPPET_SETTINGS,
    SMEP_GRIEF_REPORT,
    SMEP_DEBUG_MENU,
    SMEP_COUNT
};

class COverlayUI : public CPlanetEventHandler {
public:
    inline bool IsStartMenuOpen() { return HUDLevel.GetRef() != NULL; }
public:
    CP<RTexture> Viewfinder;
    float CurViewfinderAlpha;
    float DstViewfinderAlpha;
    CTypeinator TextChat;
    CGooeyNodeManager* OverlayUIManager;
    bool MenuNeedsUpdate;
    bool EditMode;
    CVector<SScriptedOverlay> ScriptedOverlays;
    CP<RLevel> HUDLevel;
    EPadIndex LocalPlayerRemovedWhileInPauseMenu;
    EStartMenuEntryPoint CachedStartMenuEntryPoint;
    bool RaisingPauseMenu;
};

#endif // OVERLAY_UI_H