#ifndef LOADING_SCREEN_H
#define LOADING_SCREEN_H


#include "MMString.h"

class CTextState;
typedef void (TextStateCallback)(CTextState*, u32);
class CTextState {
public:
    CTextState(int icon_mode, int title_key, int text_key, int button_mode, int progress_mode, void* userdata);
public:
    u32 title_key;
    MMString<tchar_t> title;
    u32 text_key;
    MMString<tchar_t> text;
    int button_mode;
    int icon_mode;
    int progress_mode;
    void* userdata;
    TextStateCallback* callback;
    int pad;
    float progress;
};

extern void (*UpdateButtonPrompts)();
extern void* (*SetActiveLoadingScreen)(CTextState const* text, TextStateCallback* callback, bool allow_if_other_active_screens);
extern void (*CancelActiveLoadingScreen)(void* handle, bool call_callback, u32 result);


#endif // LOADING_SCREEN_H