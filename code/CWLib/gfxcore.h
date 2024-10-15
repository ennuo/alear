#ifndef GFX_CORE_H
#define GFX_CORE_H

#include "mmtypes.h"

class CGCMTextureState {
public:
    inline CGCMTextureState()
    {
        Enable = 1;
        MinFilter = CELL_GCM_TEXTURE_LINEAR_NEAREST;
        MaxLOD = 0xf0;
        Gamma = 0;
        MagFilter = CELL_GCM_TEXTURE_LINEAR;
        WrapR = CELL_GCM_TEXTURE_WRAP;
        WrapT = CELL_GCM_TEXTURE_WRAP;
        WrapS = CELL_GCM_TEXTURE_WRAP;
        URemap = 0x0;
        ZFunc = CELL_GCM_TEXTURE_ZFUNC_LESS;
        MinLOD = 0x0;
        MaxAniso = 0x0;
        Bias = 0x0;
    }

public:
    union
    {
        u32 ControlData;
        struct
        {
            u8 Enable;
            u8 MinLOD;
            u8 MaxLOD;
            u8 MaxAniso;
        };
    };

    union 
    {
        u32 FilterData;
        struct
        {
            u16 Bias;
            u8 MinFilter;
            u8 MagFilter;
        };
    };

    union
    {
        u64 AddressData;
        struct
        {
            u8 WrapS;
            u8 WrapT;
            u8 WrapR;
            u8 URemap;
            u8 ZFunc;
            u8 Gamma;
        };
    };
};


#endif // GFX_CORE_H