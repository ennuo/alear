#ifndef POPPET_H
#define POPPET_H


#include "thing.h"

#include "ReflectionVisitable.h"
#include "PoppetEnums.inl"

// 0x14b4
class CPoppetEditState {
private:
    char Pad[0x3f8];
public:
    CThingPtr LastHoverThing;
};

class CPoppetInventory {
private:
    char Pad[0x80];
public:
    v2 SelectBoxBounds;
};

class CPoppet : public CReflectionVisitable {
public:
    void RenderHoverObject(CThing* thing, float outline);
    v2 GetBubbleSize();
    EPoppetMode GetMode() const;
    EPoppetSubMode GetSubMode() const;
    bool IsDocked() const;
    v4 GetDockPos() const;
private:
    char Pad[0x578];
public:
    CPoppetEditState Edit; // 0x580
private:
    char Pad1[0x2c];
public:
    CPoppetInventory Inventory; // 0x9b0
private:
    char Pad2[0xec0];
public:
    CThingPtr PlayerThing; // 0x1900
};


#endif // POPPET_H