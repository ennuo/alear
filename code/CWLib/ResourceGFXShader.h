#ifndef RESOURCE_GFX_SHADER_H
#define RESOURCE_GFX_SHADER_H

#include <Cg/NV/cg_types.h>

#include <vector.h>

#include "GfxPool.h"
#include "Resource.h"

class RShader : public CResource {
public:
    ByteArray programdata;
    CGprogram program;
    CGfxHandle pshadercode;
    void* vshadercode;
    const char* ConstantCache[4];
    CGparameter ParameterCache[4];
    u32 LastCacheHit;
};

class RPixelShader : public RShader {
public:

};

class RVertexShader : public RShader {
public:

};

#endif // RESOURCE_GFX_SHADER_H