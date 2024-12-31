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

class CSackBoyAnim;
class CAnimBank;

extern CVector<CEmote> gEmotes;
extern CVector<CAnimBank*> gAnimBanks;

class CAnimStyle {
public:
    inline CAnimStyle() : ID(), Bank(NULL), Gsub(0) {}
public:
    MMString<char> ID;
    CAnimBank* Bank;
    u32 Gsub;
};

struct CStyleBank {
    CVector<CAnimStyle> Styles;
};

void OnTriggerIdleAnim(CSackBoyAnim& sb);
void OnInitializeSackboyAnims(CSackBoyAnim& sb);
bool CustomInitAnims();
bool CustomInitAnimsPostResource();
namespace ScriptyStuff { int LoadAnim(CAnimBank* ab, CGUID guid); }
bool IsEmoteItem(CGUID guid);

extern CStyleBank gStyleBank;

#define E_EMOTES_KEY (3109897709)
#define E_ANIM_STYLES_KEY (3149445767)

#endif // EMOTES_H