#pragma once

#ifndef SPU
    #include <printf.h>
    #include <v2_vectorised.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <string.h>

#include <sys/types.h>
#include <sys/integertypes.h>
#include <vectormath/cpp/vectormath_aos.h>

#define ARRAY_LENGTH(x) sizeof(x) / sizeof(x[0])
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define SATURATE(x) MAX(0.0f, MIN(x, 1.0f))

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

typedef u16 tchar_t;

typedef Vectormath::Aos::Matrix3 m33;
typedef Vectormath::Aos::Quat q4;

typedef Vectormath::Aos::Vector3 v3;
typedef Vectormath::Aos::Vector4 v4;
typedef Vectormath::Aos::Matrix4 m44;

#include <GameConstants.h>
#ifdef SPU
    #include <vectormath/cpp/floatInVec.h>
    typedef Vectormath::floatInVec floatInV2;
    typedef Vectormath::Aos::Vector4 v2;

    #define FMODF(x, y) fmodf(x, y)
    #define SINF(x) sinf(x)
#else
    #include <hook.h>
    #include <refcount.h>
    
    #define FMODF(x, y) mmalex::fmod(x, y)
    #define SINF(x) mmalex::sin(x)
#endif