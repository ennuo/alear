#ifndef POPPET_INVENTORY_H
#define POPPET_INVENTORY_H

#include "PoppetChild.h"

class CPoppetInventory {
private:
    char Pad[0x80];
public:
    v2 SelectBoxBounds;
};

#endif // POPPET_INVENTORY_H