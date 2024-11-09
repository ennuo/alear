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

class CPoppet : public CReflectionVisitable {
public:
    void RenderHoverObject(CThing* thing, float outline);
    v2 GetBubbleSize();
    EPoppetMode GetMode() const;
    EPoppetSubMode GetSubMode() const;
private:
    char Pad[0x578];
public:
    CPoppetEditState Edit;
private:
    char Pad1[0xf7c];
public:
    CThingPtr PlayerThing;
};


#endif // POPPET_H