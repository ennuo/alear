#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include <cell/gcm.h>

class CRenderTarget {
enum {
    FMT_RGBA8,
    FMT_RGBAFP16,
    FMT_ZBUF,
    FMT_B8
};
public:
    CellGcmTexture Texture;
    u32 TileReg;
    u32 Format;
    u32 PPitch;
    u32 Pitch;
    u32 MSAA;
    u32 Location;
    u32 Offset;
    void* Address;
};

enum EPipelineRenderTargets {
    PRT_SHADOWMAP0=0,
    PRT_SHADOWMAP1=1,
    PRT_SHADOWMAP2=2,
    PRT_SHADOWMAPZBUF=3,
    PRT_Z_BUFFER_MSAA=4,
    PRT_N_BUFFER_MSAA=5,
    PRT_L_BUFFER_720P2X=6,
    PRT_RAY1_BUFFER_360P=7,
    PRT_RAY2_BUFFER_360P=8,
    PRT_C1_BUFFER_360P=9,
    PRT_V1_BUFFER_360P=10,
    PRT_GLOW_BUFFER_360P=11,
    PRT_BLOOM_BUFFER_256PZ=12,
    PRT_BLOOM_BUFFER_256P=13,
    PRT_BLOOM_BUFFER_128P=14,
    PRT_BLOOM_BUFFER_64P=15,
    PRT_C_BUFFER_MSAA=16,
    PRT_PODCOMPUTER=17,
    PRT_TEMP_BACKBUFFER=18,
    PRT_FLUID_DISSOLVE0=19,
    PRT_FLUID_DISSOLVE1=20,
    PRT_WATER_N=21,
    PRT_WATER_CAUSTIC=22,
    PRT_TILECACHE=23,
    PRT_MAINMEM_BOUND=24,
    PRT_MAINMEM_VEL=25,
    PRT_LAST=26
};

extern CRenderTarget gPipelineRTs[PRT_LAST];

#endif // RENDER_TARGET_H