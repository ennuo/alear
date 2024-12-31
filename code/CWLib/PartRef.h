#ifndef PART_REF_H
#define PART_REF_H

#include <refcount.h>

#include "Part.h"
#include "ResourceDescriptor.h"

class PRef : public CPart {
public:
    CResourceDescriptor<RPlan> Plan;
    CP<RPlan> PlanPtr;
    bool ChildrenSelectable;
    bool StripChildren;
    u32 OldLifetime;
    u32 OldAliveFrames;
};


#endif // PART_REF_H