#ifndef PART_MICROCHIP_H
#define PART_MICROCHIP_H

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

class PMicrochip : public CPart {
public:
    inline PMicrochip() 
    : CircuitBoardThing(), HideInPlayMode(), WiresVisible(), 
    LastTouched(-1), Offset(v4(0.0f, 500.0f, 50.0f, 0.0f)), Name(),
    Components(), CircuitBoardSizeX(420.0f), CircuitBoardSizeY(262.5f),
    KeepVisualVertical(false), BroadcastType(0)
    {}

public:
    CThingPtr CircuitBoardThing;
    bool HideInPlayMode;
    bool WiresVisible;
    int LastTouched;
    v4 Offset;
    MMString<wchar_t> Name;
    CVector<CCompactComponent> Components;
    float CircuitBoardSizeX;
    float CircuitBoardSizeY;
    bool KeepVisualVertical;
    u8 BroadcastType;
};

#endif // PART_MICROCHIP_H