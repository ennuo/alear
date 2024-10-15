#ifndef INVENTORY_ITEM_FLAGS_H
#define INVENTORY_ITEM_FLAGS_H

enum EInventoryItemFlags {
    E_IIF_NONE = 0x0,
    E_IIF_HEARTED = 0x1,
    E_IIF_UPLOADED = 0x2,
    E_IIF_CHEAT = 0x4,
    E_IIF_UNSAVED = 0x8,
    E_IIF_ERRORED = 0x10
};

#endif // INVENTORY_ITEM_FLAGS_H