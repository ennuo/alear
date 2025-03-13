#ifndef SACK_BOY_ANIM_H
#define SACK_BOY_ANIM_H

#include <vector.h>
#include "AnimBank.h"
#include "ResourceDescriptor.h"
#include "RandomStream.h"

class RMesh;
class CThing;
class CRenderYellowHead;

class CSackBoyAnim {
public:
    // actual ctor doesn't matter, just make sure everything is zero init
    inline CSackBoyAnim() { memset(this, 0, sizeof(CSackBoyAnim)); }
public:
    void InitIceData();
    void OnFreeze();
    void OnThaw();
public:
    void LoadAnims(CAnimBank* ab, bool cached);
    CRenderYellowHead* GetRenderYellowHead() const;
private:
    int GetNumFrames(int anim);
    void SetEffectMesh(RMesh* mesh);
private:
    void FreezeSolid();
    void FreezeThaw();
    void DoThawAnims();
    
    int UpdateFreezeIdleState(int last_idle);
    void OnLateAnimUpdate();
private:
    char _Pad[0x4];
public:
    CThing* Thing;
    CRandomStream NonDeterministicRand;
    CP<RGfxMaterial> ElectrocuteSkeleton;
    CResourceDescriptor<RPlan> OriginalCostumeMaterial;
private:
    char _Pad0[0x3e4];
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
public:
    s32 YANIM_DEATH_ICE_INTO;
    s32 ThawFrame;
    s32 ShiverFrame;
    bool WasFrozen;
    bool CrossedZero;
public:
    bool WasStunned;
};

#endif // SACK_BOY_ANIM_H