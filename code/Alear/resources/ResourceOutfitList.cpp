#include "resources/ResourceOutfitList.h"


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
