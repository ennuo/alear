#ifndef PART_COSTUME_H
#define PART_COSTUME_H

#include "Part.h"
#include "Resource.h"
#include "ResourceDescriptor.h"

class RMesh;
class RPlan;

class PCostume : public CPart {
public:
    void SetPowerup(RMesh* mesh, CResourceDescriptor<RPlan>& original_plan);
    void RemovePowerup(CResourceDescriptor<RPlan>& original_plan);
};


#endif // PART_COSTUME_H