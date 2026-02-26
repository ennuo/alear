#ifndef V2_VECTORISED_H
#define V2_VECTORISED_H

#include <vectormath/cpp/vectormath_aos.h>

struct vint {
    unsigned int V  __attribute__ ((__vector_size__ (16)));
};

struct vfloat {
    inline vfloat() { }
    inline vfloat(float v) { V = vec_splats(v); }
    inline vfloat(__vector float v, int slot) { V = vec_splat(v, slot); }
    inline vfloat(__vector float v) { V = v; }

    inline operator float() const { return *((float*)&V); }

    inline vfloat operator+(const vfloat& b) const { return vfloat(vec_add(V, b.V)); }
    inline vfloat operator-(const vfloat& b) const { return vfloat(vec_sub(V, b.V)); }

    __vector float V;
};

struct v2 {
    inline v2() {}
    inline v2(float v) { V = vfloat(v).V; }
    inline v2(float x, float y) { V = (__vector float){x, y, 0.0f, 0.0f}; }
    inline v2(float x, float y, float z, float w) { V = (__vector float){x, y, z, w}; }
    inline v2(__vector float v) { V = v; }

    inline v2(Vectormath::Aos::Vector4 v) { V = v.get128(); }

    inline operator Vectormath::Aos::Vector4() const { return Vectormath::Aos::Vector4(V); }

    inline vfloat operator[](unsigned int idx) { return vfloat(V, idx); }

    inline vfloat getX() const { return vfloat(V, 0); }
    inline vfloat getY() const { return vfloat(V, 1); }
    inline vfloat getZ() const { return vfloat(V, 2); }
    inline vfloat getW() const { return vfloat(V, 3); }

    inline void setX(vfloat x) { V = _vmathVfInsert(V, x.V, 0); }
    inline void setY(vfloat y) { V = _vmathVfInsert(V, y.V, 1); }
    inline void setZ(vfloat z) { V = _vmathVfInsert(V, z.V, 2); }
    inline void setW(vfloat w) { V = _vmathVfInsert(V, w.V, 3); }

    inline void setX(float x) { _vmathVfSetElement(V, x, 0); }
    inline void setY(float y) { _vmathVfSetElement(V, y, 1); }
    inline void setZ(float z) { _vmathVfSetElement(V, z, 2); }
    inline void setW(float w) { _vmathVfSetElement(V, w, 3); }
    
    inline v2 operator+(const v2& b) const { return v2(vec_add(V, b.V)); }
    inline v2 operator-(const v2& b) const { return v2(vec_sub(V, b.V)); }
    
    inline v2 operator*(float scalar) const { return *this * vfloat(scalar); }
    inline v2 operator*(vfloat scalar) const { return v2(vec_madd(V, scalar.V, (__vector float)(0.0f))); }

    __vector float V;
};

typedef vfloat floatInV2;

#endif // V2_VECTORISED_H