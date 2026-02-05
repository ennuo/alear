#ifndef PARTICLEEFFECT_H
#define PARTICLEEFFECT_H


class CParticleEffect {
public:
    static void AddFlameParticle(v2 const& a, v2 const& b, float c);
    static void AddSmokeParticle(v2 const& a, v2 const& b, float c, float d, float e, float f);
};

#endif // PARTICLEEFFECT_H