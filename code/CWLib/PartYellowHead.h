#ifndef PART_YELLOW_HEAD_H
#define PART_YELLOW_HEAD_H

#include "RenderYellowHead.h"
#include "PlayerColours.h"
#include "Part.h"

class CPoppet;
class PYellowHead : public CPart {
public:
    v4 GetActivePosition() const;
    c32 GetColour(EPlayerColour colour);
private:
    char Pad[0x30];
public:
    CRenderYellowHead* RenderYellowHead;
private:
    char Pad0[0x8];
public:
    CPoppet* Poppet;
};

#endif // PART_YELLOW_HEAD_H