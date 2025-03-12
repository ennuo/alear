#ifndef TWEAK_SHAPE_H
#define TWEAK_SHAPE_H

class CThing;

void OnStartTweaking(CThing* thing);
void OnStopTweaking(CThing* thing);

namespace TweakShapeNativeFunctions { void Register(); }

extern const u32 E_LAMS_TWEAKABLE_MATERIAL;
extern const u32 E_LAMS_TWEAKABLE_MESH;
extern const u32 E_LAMS_TWEAKABLE_DECAL;

#endif // TWEAK_SHAPE_H