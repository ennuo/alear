#ifndef POPPET_H
#define POPPET_H


#include "thing.h"

#include "ReflectionVisitable.h"

class CPoppet : public CReflectionVisitable {
private:
    char Pad[0x18f8];
public:
    CThingPtr PlayerThing;

    v2 GetBubbleSize();
};


#endif // POPPET_H