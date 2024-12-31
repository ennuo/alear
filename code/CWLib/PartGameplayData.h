#ifndef PART_GAMEPLAY_DATA_H
#define PART_GAMEPLAY_DATA_H

#include <refcount.h>

#include "EggLink.h"
#include "Part.h"

class PGameplayData : public CPart {
public:
    void SetEggLink(CP<RPlan> const& plan);
private:
    CEggLink* EggLink;
    void* KeyLink; // CKeyLink* KeyLink;
    int FluffCost;
    bool CachedHasBeenCollected;
    int CachedHasBeenCollectedFrame;
};

#endif // PART_GAMEPLAY_DATA_H