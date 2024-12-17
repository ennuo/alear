#ifndef PART_SWITCH_H
#define PART_SWITCH_H

#include <vector.h>

#include "Part.h"
#include "PlayerNumber.inl"

#include "hack_thingptr.h"

// The implementation of this file is mostly custom since I'm adding a more
// advanced switch system to LBP1, will try to make it somewhat mimick
// how it works in LBP2, but with some additional nonsense.

class CSwitchSignal {
public:
    inline CSwitchSignal() :
    Activation(0.0f), Ternary(0), Player(E_PLAYER_NUMBER_NONE)
    {}
public:
    float Activation;
    int Ternary;
    EPlayerNumber Player;
};

class CSwitchTarget {
public:
    inline CSwitchTarget() : Target(), Port()
    {}
public:
    CThingPtr Target;
    int Port;
};

class CSwitchOutput {
public:
    inline CSwitchOutput() : Activation(), TargetList(), PortThing()
    {}
public:
    CSwitchSignal Activation;
    CVector<CSwitchTarget> TargetList;
    CThingPtr PortThing;
};

class PSwitch : public CPart {
public:
    bool RaycastConnector(v4 start, v4 dir, float& t, CThing*& hit);
public:
    CVector<CThingPtr> TargetList;
private:
    char Pad[0x144];
public:
    int BehaviourOld;
    int Type;
private:
    char Pad2[0x18];
public:
    int BulletsRequired;
    int BulletsDetected;
private:
    char Pad3[0x20];
public:
    int UpdateFrame;
    int Behaviour;
    CVector<CThingPtr> InputList;
    CVector<CSwitchOutput> Outputs;
};

#endif // PART_SWITCH_H