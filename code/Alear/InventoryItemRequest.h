#ifndef INVENTORY_ITEM_REQUEST_H
#define INVENTORY_ITEM_REQUEST_H

enum EItemRequestResult {
    E_ITEM_IDLE,
    E_ITEM_PENDING,

    E_ITEM_NOT_FOUND,
    E_ITEM_NO_PLAYER,
    E_ITEM_ADDED,
    E_ITEM_ALREADY_EXISTS,
    E_ITEM_FAILED_LOAD,
    E_ITEM_NO_DATA_SOURCE,
    E_ITEM_ALREADY_IN_PROGRESS
};

void UpdateItemRequest();
EItemRequestResult RequestItem(const char* filename);
EItemRequestResult RequestItem(const void* data, u32 len);

#endif // INVENTORY_ITEM_REQUEST_H