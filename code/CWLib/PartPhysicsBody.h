#pragma once

#include <Part.h>
#include <CompactCollision.h>

#include <hack_thingptr.h>

class PBody : public CPart {
public:
struct Forked {
    v2 COM;
    CCompactMass CompactMass;
    v2 OldPosVel;
    floatInV2 OldAngVel;
};
public:
    void AddToPhysics();
    void SetFork(bool);
    
    inline const Forked& GetGame() const { return Game; }
    inline const Forked& GetRend() const { return Rend; }
    inline Forked& GetFork() const { return *Fork; }
    
    v2 GetOldPosVel() const;
    floatInV2 GetOldAngVel() const;
    void SetOldPosVel(v2);
    void SetOldAngVel(floatInV2);
    floatInV2 GetPosVelInc() const;
    v2 GetAngVelInc() const;
    floatInV2 GetPosVelMult() const;
    float GetDampeningLinear() const;
    float GetDampeningAngular() const;
    
    inline v2 GetPosVel() const
    {
        return loadxy(Fork->CompactMass.GetPosVel());
    }

    inline floatInV2 GetAngVel() const { return Fork->CompactMass.GetAngVel(); };
    inline floatInV2 GetIMass() const { return Fork->CompactMass.GetIMass(); }
    inline floatInV2 GetIMoment() const { return Fork->CompactMass.GetIMoment(); }
    
    inline v2 GetPosCOM() const { return loadxy(Fork->CompactMass.GetPos()); }
    
    bool GetFrozen() const;
    u32 FrozenBitfield() const;
    void SplatFrozenBitfield(u32);
    bool IsFrozen(int) const;
    inline bool IsMovable() const { return Frozen == 0 && GetIMass() > 0.0f; }
    bool IsRotatable() const;
    CCompactMass* GetCompactMass();
    const CCompactMass* GetCompactMass() const;
    CThing* GetEditingPlayer() const;
    bool IsImmobile() const;
    void SetEditingPlayer(CThing*);
    
    void SetPosVel(v2 v)
    {
        Fork->CompactMass.SetPosVel(v);
    }

    void SetAngVel(floatInV2);
    void SetIMass(floatInV2);
    void SetIMoment(floatInV2);
    void SetPosCOM(v2);
    void SetCOM(v2);
    void SetFrozen(int);
    void ClearFrozen(int);
    v2 GetRenderPosCOM() const;
    v2 LocalToGlobalOld(v2) const;

    v2 GetVel(v2 pos) const;
    v2 GetRenderVel(v2 pos) const;

    v2 ForceToVel(v2 pos, v2 force) const;
    void ApplyMove(v2 pos, v2 move);
    
    bool Update(u32, floatInV2);
    bool UpdateBy(v2, floatInV2, u32);
    void Initialise(bool);
    void UpdateCompactCOM();
    void SmoothPredictedPos(u32);
    void ResetPredictionSmoothing();
    void JustEmitted();
public:
    u32 Frozen;
    Forked Game;
    Forked Rend;
    Forked* Fork;
    floatInV2 PosVelInc;
    floatInV2 AngVelMult;
    v2 PrevSmoothedPosition;
    floatInV2 PrevSmoothedAngle;
    u32 PrevSmoothingFrame;
    float DampeningLinear;
    float DampeningAngular;
    CThingPtr EditingPlayer;
    u32 VisibleFrame;
    bool DelayInitialise;
    bool FullyOnScreen;
    v2 RenderPosVel;
    floatInV2 RenderAngVel;
};
