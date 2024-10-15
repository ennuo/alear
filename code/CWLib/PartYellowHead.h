#ifndef PART_YELLOW_HEAD_H
#define PART_YELLOW_HEAD_H

#include "RenderYellowHead.h"
#include "PlayerColours.h"
#include "Part.h"

class PYellowHead : public CPart {
public:
    c32 GetColour(EPlayerColour colour);
private:
    char Pad[0x30];
public:
    CRenderYellowHead* RenderYellowHead;
};

#endif // PART_YELLOW_HEAD_H