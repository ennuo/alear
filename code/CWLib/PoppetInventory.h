#ifndef POPPET_INVENTORY_H
#define POPPET_INVENTORY_H

#include <vector.h>

#include "PoppetChild.h"
#include "hack_thingptr.h"

class CPoppetInventory {
public:
    void TakePlan(CVector<CThingPtr> const& things);
private:
    char Pad[0x80];
public:
    v2 SelectBoxBounds;
};

#endif // POPPET_INVENTORY_H