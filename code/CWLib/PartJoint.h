#ifndef PART_JOINT_H
#define PART_JOINT_H

#include <vector.h>
#include "Part.h"

class PJoint : public CPart {
public:
    void SetA(CThing* thing);
    void SetB(CThing* thing);
};


#endif // PART_JOINT_H