#ifndef EMOTES_H
#define EMOTES_H

#include <vector.h>
#include <GuidHash.h>
#include <MMString.h>

class CEmoteSound {
public:
    MMString<char> Sound;
    u32 Frame;
};

class CEmote {
public:
    CVector<CEmoteSound> Sounds;
    u32 Plan;
    u32 Anim;
    int BankIndex;
};

struct CEmoteBank {
    CVector<CEmote> Emotes;
};

extern CVector<CEmote> gEmotes;

class CSackBoyAnim;
void OnTriggerIdleAnim(CSackBoyAnim& sb);
void OnInitializeSackboyAnims(CSackBoyAnim& sb);

#endif // EMOTES_H