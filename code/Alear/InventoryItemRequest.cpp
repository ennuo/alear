#include "InventoryItemRequest.h"
#include "OutfitSystem.h"

#include <cell/thread.h>
#include <GuidHashMap.h>
#include <ResourcePlan.h>
#include <ResourceGame.h>
#include <ResourceLocalProfile.h>
#include <ResourceSystem.h>
#include <thing.h>
#include <Fart.h>
#include <network/NetworkManager.h>

class CItemRequest {
public:
    EItemRequestResult Request(const void* data, u32 len)
    {
        if (HasRequest()) return E_ITEM_ALREADY_IN_PROGRESS;
        if (len == 0) return E_ITEM_NO_DATA_SOURCE;

        CHash hash;
        if (!SaveFileDataToCache(CT_TEMP, data, len,  hash)) return E_ITEM_NO_DATA_SOURCE;

        CResourceDescriptor<RPlan> desc(hash);
        Plan = LoadResource<RPlan>(desc, 0, STREAM_PRIORITY_DEFAULT, false);

        Result = E_ITEM_PENDING;
        while (Result == E_ITEM_PENDING) ThreadSleep(10);

        EItemRequestResult result = Result;
        Destroy();
        return result;
    }

    EItemRequestResult Request(const char* filename)
    {
        if (HasRequest()) return E_ITEM_ALREADY_IN_PROGRESS;

        CFileDBRow* row = FileDB::FindByPath(filename, false);
        if (row == NULL) return E_ITEM_NOT_FOUND;

        Plan = LoadResourceByKey<RPlan>(row->FileGuid.guid, 0, STREAM_PRIORITY_DEFAULT);

        Result = E_ITEM_PENDING;

        while (Result == E_ITEM_PENDING) ThreadSleep(10);

        EItemRequestResult result = Result;
        Destroy();
        return result;
    }

    inline const CP<RPlan>& GetPlan() const { return Plan; }
    inline bool HasRequest() const { return Result == E_ITEM_PENDING; }

    inline void SetResult(EItemRequestResult result)
    {
        Result = result;
    }
private:
    inline void Destroy()
    {
        Plan = NULL;
        Result = E_ITEM_IDLE;
    }
private:
    CP<RPlan> Plan;
    volatile EItemRequestResult Result;
};

CItemRequest gItemRequest;

void UpdateItemRequest()
{
    if (!gItemRequest.HasRequest()) return;

    const CP<RPlan>& plan = gItemRequest.GetPlan();

    CThing* player_thing = gGame->GetYellowheadFromPlayerNumber(gNetworkManager.InputManager.GetLocalLeadersPlayerNumber());
    if (player_thing == NULL)
    {
        gItemRequest.SetResult(E_ITEM_NO_PLAYER);
        return;
    }

    plan->BlockUntilLoaded();
    if (plan->IsError())
    {
        gItemRequest.SetResult(E_ITEM_FAILED_LOAD);
        return;
    }

    PYellowHead* yellowhead = player_thing->GetPYellowHead();

    if (yellowhead->GetLocalProfile()->AddInventoryItem(plan, 0, 0, false, false, NULL))
    {
        SpawnCollectBubble(player_thing, plan);
        gItemRequest.SetResult(E_ITEM_ADDED);
        return;
    }

    gItemRequest.SetResult(E_ITEM_ALREADY_EXISTS);
}

EItemRequestResult RequestItem(const char* filename)
{
    return gItemRequest.Request(filename);
}

EItemRequestResult RequestItem(const void* data, u32 len)
{
    return gItemRequest.Request(data, len);
}