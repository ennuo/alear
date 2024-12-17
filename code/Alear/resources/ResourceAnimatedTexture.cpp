#include "resources/ResourceAnimatedTexture.h"
#include <Serialise.h>

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;

template <typename R>
ReflectReturn Reflect(R& r, RAnimatedTexture& d)
{
    ReflectReturn ret;
    ADD(Texture);
    ADD(FrameCount);
    ADD(FrameWidth);
    ADD(FrameHeight);
    ADD(BlendFrames);

    return ret;
}

#undef ADD

template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, RAnimatedTexture& d);