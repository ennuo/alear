#include "resources/ResourceOutfitList.h"

#include <Serialise.h>


CVector<CP<ROutfitList> > gOutfitLists;

COutfit* ROutfitList::GetOutfitFromComponent(CGUID& guid)
{
    for (COutfit* it = Outfits.begin(); it != Outfits.end(); ++it)
    {
        COutfit& outfit = *it;

        for (int i = 0; i < outfit.Components.size(); ++i)
        {
            CGUID& component = outfit.Components[i];
            if (component == guid) return it;
        }
    }

    return NULL;
}

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;

template <typename R>
ReflectReturn Reflect(R& r, COutfit& d)
{
    ReflectReturn ret;
    ADD(Components);
    ADD(Outfit);
    return ret;
}

template <typename R>
ReflectReturn Reflect(R& r, ROutfitList& d)
{
    ReflectReturn ret;
    ADD(Outfits);
    return ret;
}

template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, COutfit& d);
template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, ROutfitList& d);

#undef ADD