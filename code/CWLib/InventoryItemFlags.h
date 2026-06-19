#pragma once 

enum EInventoryItemFlags {
    E_IIF_NONE = 0x0,
    E_IIF_HEARTED = 0x1,
    E_IIF_UPLOADED = 0x2,
    E_IIF_CHEAT = 0x4,
    E_IIF_UNSAVED = 0x8,
    E_IIF_ERRORED = 0x10
};

enum ECustomItemFlags {
    E_ITEM_ANIMATED = (1 << 0),
    E_ITEM_USER_COLOR = (1 << 1),
    E_ITEM_LOCKED = (1 << 2),
    E_ITEM_TEST = (1 << 3)
};