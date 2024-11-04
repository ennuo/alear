#ifndef POPPET_H
#define POPPET_H


#include "thing.h"

#include "ReflectionVisitable.h"

class CPoppet : public CReflectionVisitable {
private:
    char Pad[0x18f8];
public:
    void RenderHoverObject(CThing* thing, float outline);
    v2 GetBubbleSize();
public:
    CThingPtr PlayerThing;
};


#endif // POPPET_H