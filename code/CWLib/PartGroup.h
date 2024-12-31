#ifndef PART_GROUP_H
#define PART_GROUP_H

#include <vector.h>

#include "Part.h"
#include "ResourceDescriptor.h"
#include "network/NetworkUtilsNP.h"

class PGroup : public CPart {
public:
    inline bool HasMember(CThing* thing)
    {
        for (CThing** it = GroupMemberList.begin(); it != GroupMemberList.end(); ++it)
        {
            if (*it == thing) return true;
        }

        return false;
    }
public:
    CResourceDescriptor<RPlan> PlanDescriptor;
    NetworkPlayerID Creator;
    CRawVector<CThing*> GroupMemberList;
    CThing* Emitter;
    u32 Lifetime;
    u32 AliveFrames;
    bool Copyright;
    bool Editable;
    bool PickupAllMembers;
};


#endif // PART_GROUP_H