#pragma once

#include <vectormath/cpp/vectormath_aos.h>

static inline vec_float4 _vmathVfDot2( vec_float4 vec0, vec_float4 vec1 )
{
    vec_float4 result;
    result = vec_madd( vec0, vec1, (vec_float4)(0.0f) );
    return vec_madd( vec_sld( vec0, vec0, 4 ), vec_sld( vec1, vec1, 4 ), result );
}

struct vint {

    inline vint() { }
    vint(unsigned int, unsigned int);
    vint(unsigned int);
    vint(vec_uint4 v) : V(v) {}
    
    // unsigned int operator unsigned int() const;

    const vint operator++(int);
    const vint operator--(int);

    const vint& operator++();
    const vint& operator--();

    const vint operator-() const;

    inline const vint operator~() const
    {
        return vint(vec_nor(V, V));
    }

    vint& operator+=(vint);
    vint& operator-=(vint);

    vec_uint4 V;
};

struct vfloat {
    inline vfloat() { }
    inline vfloat(float v) { V = vec_splats(v); }
    inline vfloat(vec_float4 v, int slot) { V = vec_splat(v, slot); }
    inline vfloat(vec_float4 v) { V = v; }

    inline operator float() const { return (float)vec_extract(V, 0); }

    inline vfloat operator+(float b) const { return *this + vfloat(b); }
    inline vfloat operator+(const vfloat& b) const { return vfloat(vec_add(V, b.V)); }
    inline vfloat operator-(const vfloat& b) const { return vfloat(vec_sub(V, b.V)); }

    vec_float4 V;
};

struct v2 {
    inline v2() {}
    inline v2(float v) { V = vfloat(v).V; }
    inline v2(float x, float y) { V = (vec_float4){x, y, 0.0f, 0.0f}; }
    inline v2(float x, float y, float z, float w) { V = (vec_float4){x, y, z, w}; }
    inline v2(vec_float4 v) { V = v; }

    inline v2(Vectormath::Aos::Vector4 v) { V = v.get128(); }

    inline vfloat operator[](unsigned int idx) { return vfloat(V, idx); }

    inline vfloat getX() const { return vfloat(V, 0); }
    inline vfloat getY() const { return vfloat(V, 1); }
    inline vfloat getZ() const { return vfloat(V, 2); }
    inline vfloat getW() const { return vfloat(V, 3); }

    inline void setX(vfloat x) { V = _vmathVfInsert(V, x.V, 0); }
    inline void setY(vfloat y) { V = _vmathVfInsert(V, y.V, 1); }
    inline void setZ(vfloat z) { V = _vmathVfInsert(V, z.V, 2); }
    inline void setW(vfloat w) { V = _vmathVfInsert(V, w.V, 3); }

    void setX(v2 v);
    void setY(v2 v);
    void setZ(v2 v);
    void setW(v2 v);

    inline void setX(float x) { _vmathVfSetElement(V, x, 0); }
    inline void setY(float y) { _vmathVfSetElement(V, y, 1); }
    inline void setZ(float z) { _vmathVfSetElement(V, z, 2); }
    inline void setW(float w) { _vmathVfSetElement(V, w, 3); }
    
    inline v2 operator+(v2 b) const { return v2(vec_add(V, b.V)); }
    inline v2 operator-(v2 b) const { return v2(vec_sub(V, b.V)); }
    inline v2 operator*(v2 b) const { return v2(vec_madd(V, b.V, (vec_float4)(0.0f))); }
    inline v2 operator*(float scalar) const { return *this * vfloat(scalar); }
    inline v2 operator*(vfloat scalar) const { return v2(vec_madd(V, scalar.V, (vec_float4)(0.0f))); }
    inline v2 operator/(float scalar) const { return *this / vfloat(scalar); }
    inline v2 operator/(vfloat scalar) const { return v2(divf4(V, scalar.V)); }
    inline v2 operator/(v2 v) const { return v2(divf4(V, v.V)); }

    inline v2& operator+=(v2 v) 
    { 
        *this = *this + v;
        return *this;
    }

    inline v2& operator-=(v2 v)
    {
        *this = *this - v;
        return *this;
    }

    inline v2& operator*=(v2 v)
    {
        *this = *this * v;
        return *this;
    }

    inline v2& operator*=(vfloat scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    inline v2& operator/=(vfloat scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    inline v2 operator-() const
    {
        return v2(negatef4(V));
    }

    vfloat LengthSquared() const;
    
    inline vfloat Length3d() const
    {
        return vfloat(sqrtf4(_vmathVfDot3(V, V)), 0);
    }

    v2 Normal3d() const;
    v2 Rotate(vfloat) const;
    vfloat Angle() const;
    
    inline v2 Right() const
    {
        return vec_perm( 
            V, 
            negatef4(V),
            (vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_A, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_W }
        );
    }
    
    inline v2 Min(v2 v) const 
    { 
        return v2( vec_min( V, v.V ) ); 
    } 

    inline v2 Max(v2 v) const 
    { 
        return v2( vec_max( V, v.V )); 
    }
    
    v2 Abs() const;
    v2 CopySign(v2) const;

    inline Vectormath::Aos::Vector4 Makev4(vfloat w = (vec_float4)(1.0)) const 
    { 
        vec_float4 vec128 = V; 
        vec128 = _vmathVfInsert(vec128, w.V, 3); 
        return Vectormath::Aos::Vector4(vec128); 
    }

    inline vint operator==(v2 rhs) const
    {
        return vint((vec_uint4)vec_cmpeq(V, rhs.V));
    }

    inline vint operator!=(v2 rhs) const
    {
        return ~(*this == rhs);
    }

    inline vint operator>(v2 rhs) const
    {
        return vint((vec_uint4)vec_cmpgt(V, rhs.V));
    }

    inline vint operator<(v2 rhs) const
    {
        return vint((vec_uint4)vec_cmpgt(rhs.V, V));
    }

    inline vint operator>=(v2 rhs) const
    {
        return ~(*this < rhs);
    }
    
    inline vint operator<=(v2 rhs) const
    {
        return ~(*this > rhs);
    }

    vfloat Length() const
    {
        return vfloat(sqrtf4(_vmathVfDot2(V, V)), 0);
    }

    inline v2 Normal() const
    {
        vec_float4 dot = _vmathVfDot2(V, V);
        dot = vec_splat(dot, 0);
        return v2(vec_madd(V, rsqrtf4(dot), (vec_float4)(0.0f)));
    }
    
    v2 NormalFast() const;

    vec_float4 V;
};

typedef vfloat floatInV2;
typedef vint intInV2;

static inline v2 mergev2(v2 xy, v2 zw)
{
    const vec_uint4 mask = (vec_uint4){ 0, 0, 0xffffffff, 0xffffffff };
    return v2(vec_sel(xy.V, zw.V, mask));
}

static inline v2 loadxy(v2 v)
{
    const vec_uint4 mask = { 0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x0 };
    return v2(vec_and(v.V, (vec_float4)mask));
}

static inline vfloat dot(v2 a, v2 b)
{
    vec_float4 dot = _vmathVfDot2(a.V, b.V);
    return vfloat(dot, 0);
}

inline v2 Floor(v2 v)
{
    return v2(vec_floor(v.V));
}
