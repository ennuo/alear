#ifndef PART_RENDER_POSITION_H
#define PART_RENDER_POSITION_H

#include "Part.h"

class PPos : public CPart {
public:
    u32 AnimHash;
public:
    v4 GetBestGameplayPosv4();
};

#endif // PART_RENDER_POSITION_H