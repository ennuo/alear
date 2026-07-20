#pragma once

#include <Part.h>
#include <Input.h>
#include <NpcJumpSolve.h>

class PNpc : public CPart {
public:
    // 0x10 - JumpSolver

    // 0x84 - SackbotRecordingDataNbytes

    // 0x90 - SoundRecordingPacket
    // 0x94 - SoundRecordingPacketOffset
    // 0x98 - SackbotRecordingTimes
    
    // 0xc0 - Input
    
    // 0x308 - CThingPtr - BehaviourThing

    // 0x318 - CThingPtr - RootBehaviourThing
    // 0x324 - RecordingBehaviour
    // 0x330 - v2 - MoveTarget
    // 0x340 - int - WaitTime
    // 0x344 - u32 - Flags
    // 0x348 - EPlayerNumber - PlayerNumber

    // 0x350 - LastTimeThrown
    // 0x354 - LastTimeHitTheGround
    // 0x358 - LastThrower

    // 0x780 - ActorName



};
