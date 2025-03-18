#ifndef PART_JOINT_H
#define PART_JOINT_H

#include <vector.h>
#include "Part.h"

class PJoint : public CPart {
public:
    void SetA(PJoint* joint, CThing* thing);
    void SetB(PJoint* joint, CThing* thing);
};


#endif // PART_JOINT_H