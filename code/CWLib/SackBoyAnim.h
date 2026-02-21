#ifndef SACK_BOY_ANIM_H
#define SACK_BOY_ANIM_H

#include <vector.h>
#include "AnimBank.h"
#include "ResourceDescriptor.h"
#include "RandomStream.h"

class RMesh;
class CThing;
class CRenderYellowHead;


class CSackBoyMouthShape {
public:
    int AnimId;
};

class CSackBoyAnim {
public:
    // actual ctor doesn't matter, just make sure everything is zero init
    inline CSackBoyAnim() { memset(this, 0, sizeof(CSackBoyAnim)); }
public:
    void InitIceData();
    void OnFreeze();
    void OnThaw();
    void InitExtraAnimData();
public:
    void LoadAnims(CAnimBank* ab, bool cached);
    CRenderYellowHead* GetRenderYellowHead() const;
public:
    int GetNumFrames(int anim);
    void SetEffectMesh(RMesh* mesh);
    void DoDeathAnims();
    void Explode();
    int DoElectricDeath();
private:
    void FreezeSolid();
    void FreezeThaw();
    void DoThawAnims();
    
    int UpdateFreezeIdleState(int last_idle);
    void OnLateAnimUpdate();
public:
    void Steam(f32 size);
    void DeathSmoke(f32 size);
    void Burnilate(bool was_frozen);
    void Gasify();
    //void Electrify();
    void DoBarge(bool mirror, f32 walk_vel);
public:
    int FramesSinceExploded;
    CThing* Thing;
    CRandomStream NonDeterministicRand;
    CP<RGfxMaterial> ElectrocuteSkeleton;
    CResourceDescriptor<RPlan> OriginalCostumeMaterial;
    CP<RMesh> ExplodeMesh;
    CP<RMesh> NormalMesh;
    CP<RMesh> BitsMesh;
    CP<RMesh> BurnMesh;
    CP<RMesh> BurnFeetMesh;
    CP<RMesh> BurnHeadMesh;
    CP<RMesh> GunMesh;
    int YANIM_IDLE;
    int YANIM_IDLE_WAIT;
    int YANIM_IDLE_WALL_INTO;
    int YANIM_IDLE_WALL;
    int YANIM_IDLE_WALL_INTO_STRAIN;
    int YANIM_IDLE_HAPPY;
    int YANIM_IDLE_SAD;
    int YANIM_IDLE_ANGRY;
    int YANIM_IDLE_SCARED;
    int YANIM_YAWN;
    int YANIM_SHIFT_FEET;
    int YANIM_IDLE_FIDGET;
    int YANIM_CUSTOM_PERFUME_HERS;
    int YANIM_CUSTOM_PERFUME_HIS;
    int YANIM_CUSTOM_TORSO;
    int YANIM_CUSTOM_MOUTH;
    int YANIM_CUSTOM_HEAD;
    int YANIM_CUSTOM_HEAD_OFF;
    int YANIM_CUSTOM_TORSO_OFF;
    int YANIM_CUSTOM_WAIST;
    int YANIM_CUSTOM_HANDS;
    int YANIM_CUSTOM_HANDS_OFF;
    int YANIM_CUSTOM_HAIR;
    int YANIM_BORED_INTO;
    int YANIM_BORED_LOOP;
    int YANIM_BORED_OUTOF;
    int YANIM_IDLE_SIT_INTO;
    int YANIM_IDLE_SIT;
    int YANIM_IDLE_SIT_OUTOF;
    int YANIM_WALKSLOW;
    int YANIM_WALKMED;
    int YANIM_WALKFAST;
    int YANIM_WALKFASTSTOP;
    int YANIM_WALKFASTTURN;
    int YANIM_WALK_Z_FORWARD;
    int YANIM_WALK_Z_BACKWARD;
    int YANIM_JUMP;
    int YANIM_JUMPWALK;
    int YANIM_JUMP_SMALL;
    int YANIM_JUMP_SMALL_WALK;
    int YANIM_JUMP_GRAB;
    int YANIM_JUMPWALK_GRAB;
    int YANIM_LOOKLR;
    int YANIM_LOOKUD;
    int YANIM_LOOKROLL;
    int YANIM_REACHBR;
    int YANIM_REACHTR;
    int YANIM_REACHBC;
    int YANIM_REACHTC;
    int YANIM_REACHBRGRAB;
    int YANIM_REACHTRGRAB;
    int YANIM_REACHBCGRAB;
    int YANIM_REACHTCGRAB;
    int YANIM_REACHRGRAB;
    int YANIM_STRETCH;
    int YANIM_DRAGIDLE;
    int YANIM_PUSH_HEAVY_SLOW;
    int YANIM_PUSH_HEAVY_MEDIUM;
    int YANIM_PUSH_HEAVY_FAST;
    int YANIM_PUSH_JUMP;
    int YANIM_PUSH_LIGHT_SLOW;
    int YANIM_PUSH_LIGHT_MEDIUM;
    int YANIM_PUSH_LIGHT_FAST;
    int YANIM_PUSH_STRAIN_INTO;
    int YANIM_PUSH_STRAIN;
    int YANIM_PUSH_STRAIN_INTO_GRAB_R;
    int YANIM_PUSH_STRAIN_INTO_IDLE_WALL;
    int YANIM_PUSH_INTO_GRAB_R;
    int YANIM_GRAB_R_INTO_PUSH;
    int YANIM_GRAB_R_INTO_PUSH_STRAIN;
    int YANIM_PULL_HEAVY_SLOW;
    int YANIM_PULL_HEAVY_MEDIUM;
    int YANIM_PULL_HEAVY_FAST;
    int YANIM_PULL_JUMP;
    int YANIM_PULL_LIGHT_SLOW;
    int YANIM_PULL_LIGHT_MEDIUM;
    int YANIM_PULL_LIGHT_FAST;
    int YANIM_PULL_STRAIN_INTO;
    int YANIM_PULL_STRAIN;
    int YANIM_PULL_STRAIN_INTO_GRAB_R;
    int YANIM_PULL_STRAIN_INTO_IDLE_WALL;
    int YANIM_PULL_INTO_GRAB_R;
    int YANIM_GRAB_R_INTO_PULL;
    int YANIM_GRAB_R_INTO_PULL_STRAIN;
    int YANIM_GRAB_R_OUTOF;
    int YANIM_HANG_360_SLOW;
    int YANIM_HANG_360_FAST;
    int YANIM_HANG_360_FAST_BACKWARDS;
    int YANIM_HANG_AIR_WALK;
    int YANIM_HANG_IDLE;
    int YANIM_SLAP;
    int YANIM_SLAP_SUCCESS;
    int YANIM_SLAPPED;
    int YANIM_BLINK;
    int YANIM_JETIDLE;
    int YANIM_JETFLYRIGHT;
    int YANIM_JETJUMP;
    int YANIM_EMOTE_ANTICIPATION;
    int YANIM_EMOTE_ANGRY;
    int YANIM_EMOTE_ANGRY_TO_NEUTRAL;
    int YANIM_EMOTE_ANGRY2;
    int YANIM_EMOTE_ANGRY2_TO_NEUTRAL;
    int YANIM_EMOTE_ANGRY3;
    int YANIM_EMOTE_ANGRY3_TO_NEUTRAL;
    int YANIM_EMOTE_HAPPY;
    int YANIM_EMOTE_HAPPY_TO_NEUTRAL;
    int YANIM_EMOTE_HAPPY2;
    int YANIM_EMOTE_HAPPY2_TO_NEUTRAL;
    int YANIM_EMOTE_HAPPY3;
    int YANIM_EMOTE_HAPPY3_TO_NEUTRAL;
    int YANIM_EMOTE_SAD;
    int YANIM_EMOTE_SAD_TO_NEUTRAL;
    int YANIM_EMOTE_SAD2;
    int YANIM_EMOTE_SAD2_TO_NEUTRAL;
    int YANIM_EMOTE_SAD3;
    int YANIM_EMOTE_SAD3_TO_NEUTRAL;
    int YANIM_EMOTE_SCARED;
    int YANIM_EMOTE_SCARED_TO_NEUTRAL;
    int YANIM_EMOTE_SCARED2;
    int YANIM_EMOTE_SCARED2_TO_NEUTRAL;
    int YANIM_EMOTE_SCARED3;
    int YANIM_EMOTE_SCARED3_TO_NEUTRAL;
    int YANIM_EMOTE_SAD2_LOOP;
    int YANIM_EMOTE_SAD3_LOOP;
    int YANIM_DEATH;
    int YANIM_DEATH_FIRE_RUN;
    int YANIM_DEATH_FIRE_STUN_B_O;
    int YANIM_DEATH_FIRE_STUN_T_I;
    int YANIM_DEATH_FIRE_STUN_T_L;
    int YANIM_DEATH_FIRE_STUN_T_O;
    int YANIM_DEATH_ELECTRIC_LOOP;
    int YANIM_DEATH_ELECTRIC_O;
    int YANIM_DEATH_GAS_INTO;
    int YANIM_ACT_NEUTRAL_ARMS_SAME;
    int YANIM_ACT_HAPPY_ARMS_SAME;
    int YANIM_ACT_SAD_ARMS_SAME;
    int YANIM_ACT_ANGRY_ARMS_SAME;
    int YANIM_ACT_SCARED_ARMS_SAME;
    int YANIM_ACT_NEUTRAL_ARMS_OPP;
    int YANIM_ACT_HAPPY_ARMS_OPP;
    int YANIM_ACT_SAD_ARMS_OPP;
    int YANIM_ACT_ANGRY_ARMS_OPP;
    int YANIM_ACT_SCARED_ARMS_OPP;
    int YANIM_ACT_NEUTRAL_ARM_SMALL;
    int YANIM_ACT_HAPPY_ARM_SMALL;
    int YANIM_ACT_SAD_ARM_SMALL;
    int YANIM_ACT_ANGRY_ARM_SMALL;
    int YANIM_ACT_SCARED_ARM_SMALL;
    int YANIM_ACT_NEUTRAL_ARM_BIG;
    int YANIM_ACT_HAPPY_ARM_BIG;
    int YANIM_ACT_SAD_ARM_BIG;
    int YANIM_ACT_ANGRY_ARM_BIG;
    int YANIM_ACT_SCARED_ARM_BIG;
    int YANIM_ACT_POINT_HAND;
    int YANIM_ACT_HAPPY_HAND;
    int YANIM_ACT_SAD_HAND;
    int YANIM_CLENCHR;
    int YANIM_ACT_SCARED_HAND;
    int YANIM_HIPS_BF;
    int YANIM_HIPS_HULA;
    int YANIM_HIPS_TWIST;
    int YANIM_DECORATE_TURN_RIGHT;
    int YANIM_DECORATE_TURN_LEFT;
    int YANIM_DECORATE_TURN_180;
    int YANIM_POD_O;
    int YANIM_POD_X;
    int YANIM_POD_IDLE;
    int YANIM_POD_INTO;
    int YANIM_POD_STICK_L;
    int YANIM_POD_OUT;
    int YANIM_SPAWN;
    int YANIM_DEATH_SUICIDE_BOIL;
    int YANIM_DEATH_SUICIDE_INFLATE;
    int YANIM_DEATH_SUICIDE_INTO;
    int YANIM_DEATH_SUICIDE_OUTOF;
    int YANIM_DRYER360ANIM;
    int YANIM_DRYERRUN360ANIM;
    int YANIM_DRYERSPINANIM;
    int YANIM_DRYERRECOILRIGHTANIM;
    int YANIM_SWIM_DOWN;
    int YANIM_SWIM_DOWN_INTO;
    int YANIM_SWIM_DOWN_STROKE;
    int YANIM_SWIM_IDLE;
    int YANIM_SWIM_RIGHT;
    int YANIM_SWIM_RIGHT_INTO;
    int YANIM_SWIM_LEFT;
    int YANIM_SWIM_LEFT_INTO;
    int YANIM_SWIM_RIGHT_STROKE;
    int YANIM_SWIM_SURFACE_DIVE;
    int YANIM_SWIM_SURFACE_IDLE;
    int YANIM_SWIM_SURFACE_INTO;
    int YANIM_SWIM_SURFACE_RIGHT;
    int YANIM_SWIM_SURFACE_RIGHT_INTO;
    int YANIM_SWIM_SURFACE_RIGHT_OUTOF;
    int YANIM_SWIM_UP;
    int YANIM_SWIM_UP_INTO;
    int YANIM_SWIM_UP_STROKE;
    int YANIM_SWIM_DOWN_U_TURN;
    int YANIM_SWIM_RIGHT_U_TURN;
    int YANIM_SWIM_UP_U_TURN;
    int YANIM_SWIM_LOW_AIR;
    int YANIM_SWIM_LOW_AIR_INTO;
    int YANIM_SWIM_SURFACE_IDLE_CATCH_BREATH;
    int YANIM_SWIM_LOW_AIR_INTO_DEATH;
    int YANIM_SWIM_DEATH;
    int YANIM_SWIM_PULL_LEFT;
    int YANIM_SWIM_COLLECT_AIR;
    int PANIM_NEUTRAL;
    int PANIM_LSTICK;
    int PushIndexToPullIndex;
    float LastHorizVel;
    float WalkSpeed;
    float DampedWalkSpeed;
    float DampedDesiredWalkSpeed;
    float DampedHorizVel;
    float DampedVertVel;
    float DampedDepthVel;
    float DampedJumpiness;
    v4 Vel;
    v4 Accel;
    v4 OldPos;
    v4 OldVel;
private:
    char Pad[0x18];
public:
    int IdleFrame; // 0x418
    int LastIdle; // 0x41c
private:
    char _Pad1[0x128];
public:
    int ExpressionState; // honestly forgot which one this was     // 0x548
private:
    char _Pad2[0x48];
public:
    int DeathState;
    int DeathFrame;
    int DeathFlip;
    bool WasNormal;
    int ZapFrame;
    bool IsFrozen;
    v4 VelAtFreeze;
    int Burningness;
    int BurnFrame;
    int BurnOut;
    int BurnFlip;
    int BurnedHead;
    float BurnOutMove;
private:
    char _Pad3[0x708];
public:
    u32 IdleItemGuid; // 0xce0
private:
    char _Pad4[0xC];
public:
    int YANIM_DEATH_ICE_FROZEN;
    int YANIM_DEATH_ICE_IDLE;
    int YANIM_DEATH_ICE_WALK;
    int YANIM_DEATH_ICE_INTO;
    int YANIM_DEATH_ICE_LOOP;
    int YANIM_DEATH_ICE_OUTOF;
    s32 ThawFrame;
    s32 ShiverFrame;
    bool WasFrozen;
    bool CrossedZero;
public:
    bool WasStunned;
};

#endif // SACK_BOY_ANIM_H