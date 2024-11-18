#ifndef RESOURCE_OUTFIT_LIST_H
#define RESOURCE_OUTFIT_LIST_H

#include <vector.h>

#include <Resource.h>
#include <ResourceDescriptor.h>
#include <ResourcePlan.h>

class COutfit {
public:
    CVector<CGUID> Components;
    CResourceDescriptor<RPlan> Outfit;
};

class ROutfitList : public CResource {
public:
    inline ROutfitList(EResourceFlag flags) : CResource(flags, RTYPE_OUTFIT_LIST), Outfits()
    {
    }
public:
    u32 GetSizeInMemory() { return sizeof(ROutfitList); }
public:
    CVector<COutfit> Outfits;
};

#endif // RESOURCE_OUTFIT_LIST_H