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
    COutfit* GetOutfitFromComponent(CGUID& guid);
public:
    u32 GetSizeInMemory() { return sizeof(ROutfitList); }
public:
    CVector<COutfit> Outfits;
};

extern CVector<CP<ROutfitList> > gOutfitLists;

#include "Serialise.h"
#include "Variable.h"
template <typename R>
ReflectReturn Reflect(R& r, COutfit& d);
template <typename R>
ReflectReturn Reflect(R& r, ROutfitList& d);

#endif // RESOURCE_OUTFIT_LIST_H