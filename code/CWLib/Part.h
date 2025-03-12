#ifndef PART_H
#define PART_H

#include "ReflectionVisitable.h"
#include "PartTypeEnum.h"

class CThing;
class CPart : public CReflectionVisitable {
public:
    CThing* GetThing() const { return Thing; }
private:
    CThing* Thing;
};

#endif // PART_H