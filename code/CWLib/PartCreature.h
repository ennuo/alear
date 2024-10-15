#ifndef PART_CREATURE_H
#define PART_CREATURE_H

#include "Part.h"
#include "Input.h"
#include "GameEnums.inl"

enum EState 
{
    STATE_NORMAL            = 0,
    STATE_JETPACK           = 1,
    STATE_GUN               = 2,
    STATE_STUNNED           = 3,
    STATE_DIRECT_CONTROL    = 4,
    STATE_DEAD              = 5,
    STATE_LIMBO             = 6,
    STATE_GRAPPLE           = 7,
    STATE_BOOTS             = 8,
    STATE_FORCE             = 9,
    STATE_MOTION_CONTROLLER = 10,
    STATE_DIVER_SUIT = 11,
    STATE_GAS_MASK = 12
};

class PCreature : public CPart {
private:
    char Pad[0x928];
public:
    void SetScubaGear(bool active);
    void SetState(EState state);
    CInput* GetInput();
public:
    EState State;
};

#endif // PART_CREATURE_H