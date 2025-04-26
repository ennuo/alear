#ifndef PART_H
#define PART_H

#include "ReflectionVisitable.h"
#include "PartTypeEnum.h"

class CThing;
class CPart : public CReflectionVisitable {
public:
    CThing* GetThing() const { return Thing; }
    void SetThing_BECAUSE_I_HATE_CODING_CONVENTIONS_AND_NEED_TO_BE_SPANKED(CThing* thing) { Thing = thing; }
private:
    CThing* Thing;
};

#endif // PART_H