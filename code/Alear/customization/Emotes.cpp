#include "customization/Emotes.h"
#include "customization/Styles.h"

#include <refcount.h>
#include <cell/DebugLog.h>
#include <MMAudio.h>
#include <Variable.h>
#include <SackBoyAnim.h>
#include <Resource.h>
#include <ResourceSystem.h>
#include <ResourceSyncedProfile.h>
#include <ResourceLocalProfile.h>
#include <Poppet.h>
#include <PoppetChild.h>
#include <ResourceFileOfBytes.h>

CVector<CEmote> gEmotes;
CVector<CAnimBank*> gAnimBanks;
CStyleBank gStyleBank;

extern bool gCachedAnimLoad;
extern int gCachedAnimIndex;

namespace ScriptyStuff {
int LoadAnim(CAnimBank* ab, CGUID guid)
{
    if (gCachedAnimLoad) return gCachedAnimIndex++;

    // the actual anim bank doesn't seem to get passed to this?
    ab = gAnimBank;

    if (ab->Subst) ab->Subst->Get(guid, guid);

    int anim_index = ab->Anim.size();
    CResourceDescriptor<RAnim> desc(guid);
    CP<RAnim> anim = LoadResource<RAnim>(desc, STREAM_PRIORITY_DEFAULT, 0, false);
    ab->Anim.push_back(anim);
    return anim_index;
}
}

CAnimStyle* GetAnimStyle(const char* id)
{
    if (id == NULL) return NULL;

    for (int i = 0; i < gStyleBank.Styles.size(); ++i)
    {
        CAnimStyle& style = gStyleBank.Styles[i];
        if (strcmp(id, style.ID.c_str()) == 0)
            return &style;
    }

    return NULL;
}

CAnimStyle* GetAnimStyle(CThing* thing)
{
    if (thing == NULL) return NULL;
    PScriptName* script_name = thing->GetPScriptName();
    if (script_name == NULL) return NULL;

    char* id = script_name->Name.c_str();
    return GetAnimStyle(id);
}

bool CustomInitAnimsPostResource()
{
    for (int i = 0; i < gAnimBanks.size(); ++i)
        gAnimBanks[i]->InitPostResource();

    return true;
}

bool CustomInitAnims()
{
    if (gAnimBanks.size() != 0) return true;

    CP<RFileOfBytes> file = LoadResourceByKey<RFileOfBytes>(E_ANIM_STYLES_KEY, 0, STREAM_PRIORITY_DEFAULT);
    file->BlockUntilLoaded();
    if (!file->IsLoaded()) return false;

    CGatherVariables variables;
    variables.Init<CStyleBank>(&gStyleBank);
    if (GatherVariablesLoad(file->GetData(), variables, true, NULL) != REFLECT_OK)
    {
        DebugLog("An error occurred while loading data for animation styles!\n");
        return false;
    }

    for (int i = 0; i < gStyleBank.Styles.size(); ++i)
    {
        CAnimStyle& style = gStyleBank.Styles[i];
        style.Bank = new CAnimBank();

        if (style.Gsub) style.Bank->Subst = LoadResourceByKey<RGuidSubst>(style.Gsub, 0, STREAM_PRIORITY_DEFAULT); 

        gAnimBanks.push_back(style.Bank);
    }

    for (CAnimBank** it = gAnimBanks.begin(); it != gAnimBanks.end(); ++it)
    {
        gAnimBank = *it;

        if (gAnimBank->Subst) gAnimBank->Subst->BlockUntilLoaded();
        CSackBoyAnim sbanim;
        sbanim.LoadAnims(gAnimBank, false);
    }

    // Restore the default animation bank
    gAnimBank = gAnimBanks.front();

    return true;
}


void SetInventoryAnimationStyle(CPoppetInventory* inventory, CThing* world, RPlan* plan)
{
    // I don't think this can even happen?
    if (world == NULL) return;

    CThing* player = ((CPoppetChild*)inventory)->GetPlayer();
    PYellowHead* part = player->GetPYellowHead();
    if (part == NULL) return;

    CAnimStyle* style = GetAnimStyle(world);
    if (style == NULL) style = gStyleBank.Styles.begin(); // The first style in the array is Sackboy
    gAnimBank = style->Bank;

    const CP<RSyncedProfile>& synced_profile = part->GetSyncedProfile();
    const CP<RLocalProfile>& local_profile = part->GetLocalProfile();

    // Sync our selected animation style to our profile
    if (synced_profile)
        synced_profile->AnimationStyle = style->ID;

    // Update our last selected animation style descriptor
    if (local_profile)
    {
        CResourceDescriptor<RPlan> desc(plan->GetGUID());
        local_profile->SelectedAnimationStyle = desc;
    }

    // Detach the sackboy animation instance so it reloads the cached animations,
    // it'll be re-added in the next update frame.
    part->RenderYellowHead->RemoveSackBoyAnim();
}

bool LoadEmotes()
{
    CP<RFileOfBytes> file = LoadResourceByKey<RFileOfBytes>(E_EMOTES_KEY, 0, STREAM_PRIORITY_DEFAULT);
    file->BlockUntilLoaded();
    if (!file->IsLoaded()) return false;

    CGatherVariables variables;
    variables.Init<CEmoteBank>((CEmoteBank*)&gEmotes);
    if (GatherVariablesLoad(file->GetData(), variables, true, NULL) != REFLECT_OK)
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

// namespace ScriptyStuff {
//     MH_DefineFunc(LoadAnim, 0x000e6894, TOC0, int, CAnimBank* ab, CGUID guid);
// }

void OnInitializeSackboyAnims(CSackBoyAnim& sb)
{
    CEmote* it = gEmotes.begin();
    for (; it != gEmotes.end(); ++it)
    {
        CEmote& emote = *it;
        emote.BankIndex = ScriptyStuff::LoadAnim(gAnimBank, emote.Anim);
    }

    // if we're currently loading a cached anim set, we're probably being
    // initialized from attach script, so just set the anim bank to whatever
    // one is relevant.
    if (gCachedAnimLoad)
    {
        CThing* thing = sb.Thing;
        if (thing == NULL) return;
        PYellowHead* part = thing->GetPYellowHead();
        if (part == NULL) return;
        const CP<RSyncedProfile>& prf = part->GetSyncedProfile();
        if (prf)
            gAnimBank = GetAnimStyle(prf->AnimationStyle.c_str())->Bank;
    }
}