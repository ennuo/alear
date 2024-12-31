#include "OutfitSystem.h"

#include <hook.h>

#include <refcount.h>
#include <filepath.h>

#include <thing.h>

#include <ResourceSystem.h>
#include <ResourceFileOfBytes.h>
#include <ResourceLocalProfile.h>
#include <ResourceGame.h>
#include <PoppetEnums.inl>
#include <Player.h>

const u32 E_OUTFITS_RLST = 4052493349u;

bool LoadOutfits()
{
    CP<RFileOfBytes> rlst = LoadResourceByKey<RFileOfBytes>(E_OUTFITS_RLST, 0, STREAM_PRIORITY_DEFAULT);
    rlst->BlockUntilLoaded();

    CVector<MMString<char> > lines;
    LinesLoad(rlst->GetData(), lines, &StripAndIgnoreHash);
    for (int i = 0; i < lines.size(); ++i)
    {
        MMString<char>& line = lines[i];
        CFilePath fp(FPR_BLURAY, line.c_str());
        CP<ROutfitList> outfit_list = LoadResourceByFilename<ROutfitList>(fp, 0, STREAM_PRIORITY_DEFAULT, false);
        gOutfitLists.push_back(outfit_list);
    }

    BlockUntilResourcesLoaded((CResource**)gOutfitLists.begin(), gOutfitLists.size());
    
    return true;
}

bool DoOutfitCheck(CP<RLocalProfile>& profile, COutfit& outfit)
{
    for (int i = 0; i < outfit.Components.size(); ++i)
    {
        CGUID& guid = outfit.Components[i];
        CResourceDescriptor<RPlan> desc(guid);
        if (!profile->HasInventoryItem(desc)) return false;
    }

    CP<RPlan> plan = LoadResource<RPlan>(outfit.Outfit, STREAM_PRIORITY_DEFAULT, 0, false);
    plan->BlockUntilLoaded();

    return profile->AddInventoryItem(plan, 0, 0, false, false, NULL);
}

void OnItemCollected(CPlayer& player, CP<RPlan> const& plan)
{
    CP<RLocalProfile>& profile = player.LocalProfile;
    if (!profile) return;

    if (!plan || !plan->IsLoaded()) return;
    
    if ((plan->InventoryData.Type & E_TYPE_COSTUME) == 0) return;

    CGUID& guid = plan->GetGUID();
    if (!guid) return;

    for (int i = 0; i < gOutfitLists.size(); ++i)
    {
        CP<ROutfitList>& outfit_list = gOutfitLists[i];
        if (!outfit_list || !outfit_list->IsLoaded()) continue;
        COutfit* outfit = outfit_list->GetOutfitFromComponent(guid);
        if (outfit == NULL) continue;
        if (DoOutfitCheck(profile, *outfit))
        {
            CThing* player_thing = gGame->GetYellowheadFromPlayerNumber(player.PlayerNumber);

            CP<RPlan> outfit_plan = LoadResource<RPlan>(outfit->Outfit, STREAM_PRIORITY_DEFAULT, 0, false);
            outfit_plan->BlockUntilLoaded();

            SpawnCollectBubble(player_thing, outfit_plan);
        }
    }
}

void SpawnCollectBubble(CThing* player, CP<RPlan> const& plan)
{
    if (player == NULL || !plan || !plan->IsLoaded()) return;
    PYellowHead* yellowhead = player->GetPYellowHead();
    if (yellowhead == NULL) return;

    // I don't think we need to add it to a world or anything,
    // we just need the parts needed to make sure the CollectItem
    // function doesn't crash.
    CThing* egg = new CThing();
    egg->AddPart(PART_TYPE_POS);
    egg->AddPart(PART_TYPE_GAMEPLAY_DATA);

    // Make sure our fake egg technically exists wherever the player curently is
    egg->GetPPos()->SetWorldPos(player->GetPPos()->Game.WorldPosition, false, 0);
    egg->GetPGameplayData()->SetEggLink(plan);

    yellowhead->CollectItem(egg, true);

    // Don't leak our egg!
    delete egg;
}

extern "C" uintptr_t _outfitlist_onitemadded;
void InitOutfitHooks()
{
    MH_PokeBranch(0x0040c818, &_outfitlist_onitemadded);
}