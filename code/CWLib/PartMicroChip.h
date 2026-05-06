#pragma once

#include <vector.h>
#include <MMString.h>

#include <Part.h>
#include <hack_thingptr.h>

class CCompactComponent {
public:
    inline CCompactComponent() : Thing(), X(), Y(), Angle(), ScaleX(1.0f), ScaleY(1.0f), Flipped(false)
    {}
public:
    CThingPtr Thing;
    float X, Y, Angle;
    float ScaleX, ScaleY;
    bool Flipped;
};


class PMicroChip : public CPart {
public:
    inline PMicroChip() 
    : CircuitBoardThing(), HideInPlayMode(), WiresVisible(), X(),
    LastTouched(-1), Offset(v4(0.0f, 500.0f, 50.0f, 1.0f)), Name(),
    Components(), CircuitBoardSizeX(420.0f), CircuitBoardSizeY(262.5f),
    KeepVisualVertical(false), BroadcastType(0),
    AnimationCurrent(), AnimationDelta(), AnimationTarget(), Y(), Z(), W(),
    Y0(), Y1()
    {}
    
    ~PMicroChip();
public:
    void SetCircuitBoardVisible(bool visible);
    bool IsCircuitBoardVisible() const;
    void CreateCircuitBoard();
    void ShowCircuitBoard(bool visible);
    void UpdateAnimation();
    void UpdateRenderPos();
    void UpdateRenderDepths();
    inline bool IsSequencer() const { return false; }
public:
    CThingPtr CircuitBoardThing;
    bool HideInPlayMode;
    bool WiresVisible;
    bool X;
    int LastTouched;
    v4 Offset;
    float AnimationCurrent;
    float AnimationDelta;
    float AnimationTarget;
    float CircuitBoardSizeX;
    float CircuitBoardSizeY;
    int Y;
    int Z;
    int W;
    const wchar_t* Name;
    int Y0;
    int Y1;
    CVector<CCompactComponent>* Components;
    bool KeepVisualVertical;
    u8 BroadcastType;
};
