#include "customization/Styles.h"
#include "customization/SlapStyles.h"
#include "customization/PoppetStyles.h"
#include "customization/Emotes.h"


#include <ppcasm.h>

extern "C" void _beslapped_hook();
extern "C" void _sbanim_emote_init_hook();
extern "C" void _sbanim_update_emote_sounds_hook();
extern "C" void _emote_hook();
extern "C" void _emote_select_hook();
extern "C" void _popit_update_menu_shape_hook();
extern "C" void _animstyles_hook();
extern "C" void _get_slap_force_hook();
extern "C" void _sbanim_late_update_hook();

MH_DefineFunc(CSackBoyAnim_Destructor, 0x000f1fe0, TOC0, void, CSackBoyAnim*);
void DestroyAllocatedInstance(CSackBoyAnim* anim)
{
    // Have to make sure these get unset
    // or else the game is going to crash at some point.
    anim->JustLaunchedOff = NULL;
    anim->LastLaunchedOff = NULL;

    // Call normal destructor now, this function is only
    // used on the allocated version of this class.
    CSackBoyAnim_Destructor(anim);
}

void InitEmoteHooks()
{
    MH_Poke32(0x0038a468, B(&_emote_select_hook, 0x0038a468));
    MH_Poke32(0x000ec724, B(&_sbanim_update_emote_sounds_hook, 0x000ec724));
    MH_Poke32(0x000fa94c, B(&_sbanim_emote_init_hook, 0x000fa94c));
    MH_Poke32(0x000f6130, B(&_emote_hook, 0x000f6130));

    MH_InitHook((void*)0x000e6454, (void*)&CustomInitAnims);
    MH_InitHook((void*)0x000e6894, (void*)&ScriptyStuff::LoadAnim);
    MH_InitHook((void*)0x000e6858, (void*)&CustomInitAnimsPostResource);

    MH_PokeCall(0x000e5c60, DestroyAllocatedInstance);
    MH_PokeBranch(0x000fed60, &_sbanim_late_update_hook);
    MH_PokeBranch(0x0038ae78, &_animstyles_hook);
}

void InitSlapStyleHooks()
{
    MH_Poke32(0x000e6d88, B(&_beslapped_hook, 0x000e6d88));
    MH_PokeBranch(0x000738c4, &_get_slap_force_hook);
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
