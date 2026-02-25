#ifndef FLUIDRENDER_H
#define FLUIDRENDER_H


class CFluidRender {
public:
    static void AddPaintSplatBits(m44 pos, v4 color);
    static void AddExplosionBits(m44 pos);
    static void AddFluidBlob(v4 position, v4 velocity, u32 player, v4 color, v4 density, u32 color_frame, float color_radius);
};

#endif // FLUIDRENDER_H