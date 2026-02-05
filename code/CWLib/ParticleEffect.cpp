#include "ParticleEffect.h"

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>

MH_DefineFunc(CParticleEffect_AddFlameParticle, 0x001d6a4c, TOC0, void, v2 const&, v2 const&, float);
MH_DefineFunc(CParticleEffect_AddSmokeParticle, 0x001d602c, TOC0, void, v2 const&, v2 const&, float, float, float, float);

void CParticleEffect::AddFlameParticle(v2 const& a, v2 const& b, float c)
{
    CParticleEffect_AddFlameParticle(a, b, c);
}
void CParticleEffect::AddSmokeParticle(v2 const& a, v2 const& b, float c, float d, float e, float f)
{
    CParticleEffect_AddSmokeParticle(a, b, c, d, e, f);
}