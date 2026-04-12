#pragma once

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
    COutfit* GetOutfitFromComponent(CGUID& guid);
public:
    u32 GetSizeInMemory() { return sizeof(ROutfitList); }
public:
    CVector<COutfit> Outfits;
};

extern CVector<CP<ROutfitList> > gOutfitLists;
