#include "customization/Styles.h"
#include "customization/SlapStyles.h"
#include "customization/PoppetStyles.h"
#include "customization/Emotes.h"

#include <hook.h>
#include <ppcasm.h>

extern "C" void _beslapped_hook();
extern "C" void _sbanim_emote_init_hook();
extern "C" void _sbanim_update_emote_sounds_hook();
extern "C" void _emote_hook();
extern "C" void _emote_select_hook();
extern "C" void _popit_update_menu_shape_hook();
extern "C" void _animstyles_hook();

void InitEmoteHooks()
{
    MH_Poke32(0x0038a468, B(&_emote_select_hook, 0x0038a468));
    MH_Poke32(0x000ec724, B(&_sbanim_update_emote_sounds_hook, 0x000ec724));
    MH_Poke32(0x000fa94c, B(&_sbanim_emote_init_hook, 0x000fa94c));
    MH_Poke32(0x000f6130, B(&_emote_hook, 0x000f6130));

    MH_InitHook((void*)0x000e6454, (void*)&CustomInitAnims);
    MH_InitHook((void*)0x000e6894, (void*)&ScriptyStuff::LoadAnim);
    MH_InitHook((void*)0x000e6858, (void*)&CustomInitAnimsPostResource);

    MH_PokeBranch(0x0038ae78, &_animstyles_hook);
}

void InitSlapStyleHooks()
{
    MH_Poke32(0x000e6d88, B(&_beslapped_hook, 0x000e6d88));
}

void InitPopitStyleHooks()
{
    MH_InitHook((void*)0x0029a768, (void*)&OnFillInfoBubbleBackground);
    MH_InitHook((void*)0x00344084, (void*)&OnFillPoppetBackground);
    MH_InitHook((void*)0x00343dc4, (void*)&CustomGetBubbleSize);
    MH_Poke32(0x0035b410, B(&_popit_update_menu_shape_hook, 0x0035b410));
}

void InitStyleHooks()
{
    InitPopitStyleHooks();
    InitSlapStyleHooks();
    InitEmoteHooks();
}
