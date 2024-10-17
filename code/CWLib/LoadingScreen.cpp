#include "LoadingScreen.h"
#include "hook.h"

// < 0xa = also running sackboy?
// 0xb = running sackboy
// 0xc = earth
// 0xd = question mark?
// 0xe = document with signature
// 0xf = loading/saving icon
// 0x10 = sackboy running frozen frame, probably invalid?

// 0 = NONE (NARROW)
// 1 = NONE (WIDE)
// 2 = ACCEPT (X) DECLINE (O)
// 3 = YES (X) NO (O)
// 4 = OK (X) CANCEL (0)
// 5 = OK (X)
// 6 = CANCEL (O)
// 7 = DELETE (X) RETRY (O)
// 8 = DELETE (X) CANCEL (O)
// 9 = TRY BACKUP (X) RETRY (O)
// 10 = (X) (O) (probably invalid state)

MH_DefineFunc(UpdateButtonPrompts, 0x00265b3c, TOC0, void);

MH_DefineFunc(SetActiveLoadingScreen, 0x00265624, TOC0, void*, CTextState const* text, TextStateCallback* callback, bool allow_if_other_active_screens);
MH_DefineFunc(CancelActiveLoadingScreen, 0x00267dc8, TOC0, void, void* handle, bool call_callback, u32 result);

MH_DefineFunc(CTextState_CTextState, 0x002652e4, TOC0, void, CTextState*, int, int, int, int, int, void*);
CTextState::CTextState(int icon_mode, int title_key, int text_key, int button_mode, int progress_mode, void* userdata)
{
    CTextState_CTextState(this, icon_mode, title_key, text_key, button_mode, progress_mode, userdata);
}