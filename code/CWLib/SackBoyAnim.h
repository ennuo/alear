#ifndef SACK_BOY_ANIM_H
#define SACK_BOY_ANIM_H

#include <vector.h>
#include "AnimBank.h"

class RMesh;
class CThing;

class CSackBoyAnim {
private:
    void FreezeSolid();
    void SetEffectMesh(RMesh* mesh);
private:
    char _Pad[0x4];
public:
    CThing* Thing;
private:
    char _Pad0[0x414];
public:
    u32 LastIdle; // 0x41c
private:
    char _Pad1[0x128];
public:
    int ExpressionState; // honestly forgot which one this was     // 0x548
private:
    char _Pad2[0x794];
public:
    u32 IdleItemGuid; // 0xce0
private:
    char _Pad3[0xC];
};

#endif // SACK_BOY_ANIM_H