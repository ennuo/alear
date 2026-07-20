#pragma once

class CCompactMass;

class CCompactCollidability {
public:
    u16 CollisionBits;
    u8 CollideType;
    u8 CollideIgnore;
    CCompactMass* Mass;
};
