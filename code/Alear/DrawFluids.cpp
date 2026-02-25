#include "DrawFluids.h"
#include "FluidRender.h"
#include "ParticleEffect.h"

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>

#include <cell/DebugLog.h>
    
namespace DrawFluidsNativeFunctions
{
    void TriggerPaintSplat(m44 pos, v4 color)
    {
        CFluidRender* fluid_render;
        fluid_render->AddPaintSplatBits(pos, color);
    }
    void TriggerExplosion(m44 pos)
    {
        CFluidRender* fluid_render;
        fluid_render->AddExplosionBits(pos);
    }
    void TriggerFluidBlob(v4 position, v4 velocity, u32 player, v4 color, v4 density, u32 color_frame, f32 color_radius)
    {
        DebugLog("FLUID RENDERED!!!");
        CFluidRender* fluid_render;
        fluid_render->AddFluidBlob(position, velocity, player, color, density, color_frame, color_radius);
    }
    void TriggerFlameParticle(v2 a, v2 b, f32 c)
    {
        DebugLog("FLAME RENDERED!!!");
        CParticleEffect* particle_effect;
        particle_effect->AddFlameParticle(a, b, c);
    }
    void TriggerSmokeParticle(v2 a, v2 b, f32 c, f32 d, f32 e, f32 f)
    {
        DebugLog("SMOKE RENDERED!!!");
        CParticleEffect* particle_effect;
        particle_effect->AddSmokeParticle(a, b, c, d, e, f);
    }
    
    void Register()
    {
        RegisterNativeFunction("TriggerCollectAmmo", "TriggerExplosion__m", true, NVirtualMachine::CNativeFunction1V<m44>::Call<TriggerExplosion>);
        RegisterNativeFunction("TriggerCollectAmmo", "TriggerPaintSplat__mr", true, NVirtualMachine::CNativeFunction2V<m44, v4>::Call<TriggerPaintSplat>);
        RegisterNativeFunction("TriggerCollectAmmo", "TriggerFluidBlob__rrirrif", true, NVirtualMachine::CNativeFunction7V<v4, v4, u32, v4, v4, u32, f32>::Call<TriggerFluidBlob>);
        RegisterNativeFunction("TriggerCollectAmmo", "TriggerFlameParticle__ppf", true, NVirtualMachine::CNativeFunction3V<v2, v2, f32>::Call<TriggerFlameParticle>);
        RegisterNativeFunction("TriggerCollectAmmo", "TriggerSmokeParticle__ppffff", true, NVirtualMachine::CNativeFunction6V<v2, v2, f32, f32, f32, f32>::Call<TriggerSmokeParticle>);
    }
}