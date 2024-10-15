#ifndef COLOUR_H
#define COLOUR_H



class c32 {
public:
    inline void GetRGBAf(float& r, float& g, float& b, float& a)
    {
        r = (float)((Bits >> 16) & 0xFF) / 255.0f;
        g = (float)((Bits >> 8) & 0xFF) / 255.0f;
        b = (float)((Bits >> 0) & 0xFF) / 255.0f;
        a = (float)((Bits >> 24) & 0xFF) / 255.0f;
    }

    inline void GetRGBAf(float* col)
    {
        GetRGBAf(col[0], col[1], col[2], col[3]);
    }
public:
    u32 Bits;
};

#endif // COLOUR_H