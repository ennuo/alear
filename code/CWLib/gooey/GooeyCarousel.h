#ifndef GOOEY_CAROUSEL_H
#define GOOEY_CAROUSEL_H

#include <refcount.h>
#include <vector.h>
#include <MMString.h>

#include "ResourceGFXTexture.h"
#include "gooey/GooeyNodeLeaf.h"
#include "gooey/GooeyContainerStyles.inl"
#include "GooeyNetworkAction.h"

class CCarouselItem {
public:
    inline CCarouselItem() : ToolTip(), Icon(), UV(0.0f, 0.0f, 0.0f, 1.0f)
    {}

    inline CCarouselItem(RTexture* icon, v4 uv, u32 lams_key, v4 tint) : ToolTip(), Icon(icon), UV(uv)
    {

    }

    inline CCarouselItem(RTexture* icon, v4 uv, const wchar_t* tooltip, v4 tint) : ToolTip(tooltip), Icon(icon), UV(uv)
    {

    }
public:
    MMString<wchar_t> ToolTip;
    CP<RTexture> Icon;
    v4 UV;
};

class CGooeyCarousel : public CGooeyNodeLeaf {
class CarouselAttributes : public NodeAttributes {
    float ItemScrollX;
    v2 ItemSize;
};
public:
    u32 LastRightStickFrame;
    s32 PreviousIndex;
    s32 CurrentIndex;
    s32 BiggestIndex;
    s32 SmallestIndex;
    ECarouselType CarouselType;
    CVector<CCarouselItem> ItemList;
    StateZoomer<CarouselAttributes> State;
    NodeExtra StateExtra;
    EGooeyNetworkAction NetworkAction;
    int NetworkActionThingUID;
};

#endif // GOOEY_CAROUSEL_H