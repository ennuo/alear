#ifndef PART_RENDER_POSITION_H
#define PART_RENDER_POSITION_H

#include "Part.h"
#include "hack_thingptr.h"

class PPos : public CPart {
struct Forked {
    m44 LocalPosition;
    m44 WorldPosition;
    m44 InvWorldPosition;
};
public:
    v4 GetBestGameplayPosv4();
    void SetWorldPos(m44 const& m, bool recompute_stickers, unsigned int);
public:
    u32 AnimHash;
public:
    Forked Game;
    Forked Rend;
    Forked* Fork;
    CThingPtr ThingOfWhichIAmABone;
};

#endif // PART_RENDER_POSITION_H