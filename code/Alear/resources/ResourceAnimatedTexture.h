#ifndef RESOURCE_ANIMATED_TEXTURE_H
#define RESOURCE_ANIMATED_TEXTURE_H


#include "refcount.h"
#include "ResourceGFXTexture.h"
#include "Resource.h"

#include <cell/DebugLog.h>

class RAnimatedTexture : public CResource {
public:
    inline RAnimatedTexture(EResourceFlag flags) : CResource(flags, RTYPE_ANIMATED_TEXTURE),
    Texture(), FrameCount(), FrameWidth(), FrameHeight(), BlendFrames()
    {
        
    }
public:
    inline v4 GetUV(u32 frame)
    {
        Texture->BlockUntilLoaded();
        if (!Texture->IsLoaded() || FrameCount <= 1) return v4(0.0f, 0.0f, 1.0f, 1.0f);

        if (frame < 0) frame = 0;
        frame %= FrameCount;

        u32 num_columns = Texture->gcmtexture.width / FrameWidth;
        u32 num_rows = Texture->gcmtexture.height / FrameHeight;
        
        f32 x = (f32)(frame % num_columns);
        f32 y = (f32)(frame / num_columns);

        return v4(
            x / ((f32) num_columns),
            y / ((f32) num_rows),
            (x + 1.0f) / ((f32) num_columns),
            (y + 1.0f) / ((f32) num_rows)
        );
    }
public:
    u32 GetSizeInMemory() { return sizeof(RAnimatedTexture); }
public:
    CP<RTexture> Texture;
    u32 FrameCount;
    u32 FrameWidth;
    u32 FrameHeight;
    bool BlendFrames;
};

#include "Serialise.h"
#include "Variable.h"
template <typename R>
ReflectReturn Reflect(R& r, RAnimatedTexture& d);

#endif // RESOURCE_ANIMATED_TEXTURE_H