#pragma once

const u32 PRNG_A = 0x02e90edd;
const u32 PRNG_B = 0x0e908fc1;
const u32 PRNG_M = 0x40000000;
const u32 PRNG_M_MASK = 0x3fffffff;
const float PRNG_INV_DENOM = 6.10388815402985E-5;

class CRandomStream {
public:
    CRandomStream(u32 seed = 42) : r0(seed)
    {
        
    }
public:
    inline void SetSeed(u32 seed)
    {
        r0 = seed;
    }

    inline u32& GetSeedForSerialisation() { return r0; }

    inline s32 GetInt()
    {
        r0 = r0 * PRNG_A + PRNG_B & PRNG_M_MASK;
        return r0 >> 16;
    }

    inline u32 GetBigInt()
    {
        r0 = r0 * PRNG_A + PRNG_B & PRNG_M_MASK;
        return r0;
    }

    inline s32 GetInt(s32 min, s32 max)
    {
        return ((GetInt() * (max - min)) >> 14) + min;
    }
    
    inline float GetFloat()
    {
        return GetInt() * PRNG_INV_DENOM;
    }

    inline float GetFloat(float min, float max)
    {
        return GetFloat() * (max - min) + min;
    }
    
    v3 GetUnitVector();
    v2 GetUnitVectorv2();
private:
    u32 r0;
};
