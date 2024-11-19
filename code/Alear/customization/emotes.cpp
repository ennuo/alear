#include "customization/emotes.h"
#include "customization/styles.h"

#include <cell/DebugLog.h>
#include <MMAudio.h>
#include <Variable.h>
#include <SackBoyAnim.h>
#include <Resource.h>

CVector<CEmote> gEmotes;
CVector<CAnimBank*> gAnimBanks;

bool LoadEmotes()
{
    CFilePath fp(FPR_GAMEDATA, "gamedata/alear/data/emotes.txt");
    if (!FileExists(fp))
    {
        DebugLog("Skipping load of emotes, since no configuration file exists at %s\n", fp.c_str());
        return true;
    }

    ByteArray b; CHash hash;
    if (!FileLoad(fp, b, hash))
    {
        DebugLog("An error occurred reading configuration file for emotes!!\n");
        return false;
    }

    CGatherVariables variables;
    variables.Init<CEmoteBank>((CEmoteBank*)&gEmotes);
    if (GatherVariablesLoad(b, variables, true, NULL) != REFLECT_OK)
    {
        DebugLog("An error occurred while loading data for emotes!\n");
        return false;
    }
    
    CEmote* it = gEmotes.begin();
    for (; it != gEmotes.end(); ++it)
    {
        CEmote& emote = *it;
        DebugLog("CEmote:\n\tPlan: %08x\n\tAnim: %08x\n", emote.Plan, emote.Anim);
        for (int i = 0; i < emote.Sounds.size(); ++i)
        {
            CEmoteSound& sound = emote.Sounds[i];
            DebugLog("\tCEmoteSound\n\t\tSound: %s\n\t\tFrame: %d\n", sound.Sound.c_str(), sound.Frame);
        }
    }

    return true;
}

void UnloadEmotes()
{
    return;
}

bool IsEmoteItem(CGUID guid)
{
    for (CEmote* it = gEmotes.begin(); it != gEmotes.end(); ++it)
    {
        if (it->Plan == guid.guid)
            return true;
    }

    return false;
}

void OnTriggerIdleAnim(CSackBoyAnim& sb)
{
    if (sb.IdleItemGuid == 0) return;
    
    CEmote* it = gEmotes.begin();
    for (; it != gEmotes.end(); ++it)
    {
        CEmote& emote = *it;
        if (emote.Plan == sb.IdleItemGuid)
        {
            sb.LastIdle = emote.BankIndex;
            break;
        }
    }
}

void OnUpdateAnimationSounds(CSackBoyAnim& sb, int anim, int frame)
{
    if (sb.Thing == NULL) return;

    CEmote* emote = NULL;
    for (CEmote* it = gEmotes.begin(); it != gEmotes.end(); ++it)
    {
        if (it->BankIndex == anim)
        {
            emote = it;
            break;
        }
    }

    if (emote == NULL || emote->Sounds.size() == 0) return;

    // Play any animation sounds if they exist at the current frame
    for (CEmoteSound* it = emote->Sounds.begin(); it != emote->Sounds.end(); ++it)
    {
        CEmoteSound& sound = *it;
        if (sound.Frame == frame)
            CAudio::PlaySample(CAudio::gSFX, sound.Sound.c_str(), sb.Thing, -10000.0f, -10000.0f);
    }
}

namespace ScriptyStuff {
    MH_DefineFunc(LoadAnim, 0x000e6894, TOC0, int, CAnimBank* ab, CGUID guid);
}

void OnInitializeSackboyAnims(CSackBoyAnim& sb)
{
    CEmote* it = gEmotes.begin();
    for (; it != gEmotes.end(); ++it)
    {
        CEmote& emote = *it;

        // Check if the animation is already in the bank
        int anim_index = -1;
        // CVector<CP<RAnim> >& anim_list = gAnimBank->Anim;
        // for (int i = 0; i < anim_list.size(); ++i)
        // {
        //     CP<RAnim>& anim = anim_list[i];
        //     if (anim->GetGUID().guid == emote.Anim)
        //     {
        //         anim_index = i;
        //         break;
        //     }
        // }

        if (anim_index == -1)
            anim_index = ScriptyStuff::LoadAnim(gAnimBank, emote.Anim);

        emote.BankIndex = anim_index;
    }
}