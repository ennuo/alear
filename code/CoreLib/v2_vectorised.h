#ifndef V2_VECTORISED_H
#define V2_VECTORISED_H

struct vint {
    unsigned int V  __attribute__ ((__vector_size__ (16)));
};

struct vfloat {
    inline vfloat() : V() { }
    inline vfloat(float v) { V = (__vector float){v, v, v, v}; }
    inline vfloat(__vector float v) { V = v; }

    inline operator float() const { return (float)V[0]; }

    inline vfloat operator+(const vfloat& b) const { return V + b.V; }
    inline vfloat operator-(const vfloat& b) const { return V - b.V; }

    __vector float V;
};

struct v2 {
    inline v2() { V = (__vector float){0.0f, 0.0f, 0.0f, 0.0f}; }
    inline v2(float v) { V = (__vector float){v, v, v, v}; }
    inline v2(float x, float y) { V = (__vector float){x, y, x, y}; }
    inline v2(__vector float v) { V = v; }

    inline vfloat getX() { return V[0]; }
    inline vfloat getY() { return V[1]; }
    inline vfloat getZ() { return V[2]; }
    inline vfloat getW() { return V[3]; }

    // inline void setX(vfloat x) { V[0] = x.V[0]; }
    // inline void setY(vfloat y) { V[1] = y.V[0]; }
    // inline void setZ(vfloat z) { V[2] = z.V[0]; }
    // inline void setW(vfloat w) { V[3] = w.V[0]; }

    // inline void setX(v2 v) { V[0] = v.V[0]; }
    // inline void setY(v2 v) { V[1] = v.V[1]; }
    // inline void setZ(v2 v) { V[2] = v.V[2]; }
    // inline void setW(v2 v) { V[3] = v.V[3]; }

    inline v2 operator+(const v2& b) const { return V + b.V; }
    inline v2 operator-(const v2& b) const { return V - b.V; }
    inline v2 operator*(const v2& b) const { return V * b.V; }
    inline v2 operator/(const v2& b) const { return V / b.V; }

    float V __attribute__ ((__vector_size__ (16)));
};

#endif // V2_VECTORISED_H