#ifndef TWEAK_SHAPE_H
#define TWEAK_SHAPE_H

class CThing;

void OnStartTweaking(CThing* thing);
void OnStopTweaking(CThing* thing);

namespace TweakShapeNativeFunctions { void Register(); }

#endif // TWEAK_SHAPE_H