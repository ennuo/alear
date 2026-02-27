#ifndef PART_TRIGGER_H
#define PART_TRIGGER_H

#include <refcount.h>

#include "GameEnums.inl"
#include "Part.h"

enum ETriggerType {
    TR_RADIUS,
    TR_RECT,
    TR_SWITCH,
    TR_TOUCH,
    TR_3DRADIUS,

    TR_MAX
};

enum ETriggerUpdateResult {
    TRIGGER_UPDATE_RESULT_STILL_ALIVE,
    TRIGGER_UPDATE_RESULT_DELETED
};

class PTrigger : public CPart {
public:
    u8 TriggerType;
    bool Enabled;
    bool AllZLayers;
};

#endif // PART_TRIGGER_H