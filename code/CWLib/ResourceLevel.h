#ifndef RESOURCE_LEVEL_H
#define RESOURCE_LEVEL_H


#include "thing.h"
#include "Resource.h"
#include "PlayerNumber.inl"
#include "network/NetworkUtilsNP.h"

class CPlayerRecord {
protected:
    NetworkPlayerID PlayerIDs[32];
    EPlayerNumber PlayerNumbers[32];
    u32 Offset;
};

class RGame;
class RLevel : public CResource {
public:
    inline PWorld* GetWorld() 
    {
        if (WorldThing.GetThing() != NULL)
            return WorldThing->GetPWorld();
        return NULL;
    }
public:
    CThingPtr WorldThing;
    bool FixedUp;
    CPlayerRecord PlayerRecord;
    RGame* Game;
    bool UseNewCheckpointCode;
};

#endif // RESOURCE_LEVEL_H