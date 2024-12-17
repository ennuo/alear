#ifndef GOOEY_IMAGE_H
#define GOOEY_IMAGE_H

#include <resources/ResourceAnimatedTexture.h>

#include "gooey/GooeyNodeLeaf.h"
#include "FontEnum.h"

class CGooeyImage : public CGooeyNodeLeaf {
class ImageAttributes : public NodeAttributes {
    v4 SunburstRoundyTextString;
};
public:
    void UpdateAnimatedTexture();
    void InitSDF(CP<CResource>& resource, u32 sdf_flags, v2 size, v4 colour, v4 uvs);
    void InitSDF(CP<RTexture>& texture, u32 sdf_flags, v2 size, v4 colour, v4 uvs);
    void InitializeExtraData();
protected:
    CP<RTexture> SDFTexture;
    CP<RTexture> Texture;
    u32 SDFFlags;
    EGooeyIcon IconType;
    v4 UVs;
    StateZoomer<ImageAttributes> State;
    NodeExtra StateExtra;
    MMString<wchar_t> OverlayString;
    EFontList OverlayFont;
    bool HasBorder;
    float BorderSize;
private:
    char Pad[0x20];
protected:
    // Custom fields for generic animated texture support
    CP<RAnimatedTexture> AnimatedTexture;
    u32 FrameDelay;
    u32 FrameTimer;
    u32 CurrentFrame;
};

#endif // GOOEY_IMAGE_H