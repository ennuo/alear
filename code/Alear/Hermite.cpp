#include <Hermite.h>

#define PHI0(t)  (2.0f * t * t * t - 3.0f * t * t + 1.0f)
#define PHI1(t)  (t * t * t - 2.0f * t * t + t)
#define PSI0(t)  (-2.0f * t * t * t + 3.0f * t * t)
#define PSI1(t)  (t * t * t - t * t)

int Hermite(v4* out_points, int num_points, v4 p0, v4 p1, float v0x, float v0y, float v1x, float v1y)
{
    float p0x = p0.getX();
    float p0y = p0.getY();
    float p1x = p1.getX();
    float p1y = p1.getY();

    const float HERMITE_PRECISION = 1.0f / num_points;

    int i = 0;
    for (float t = 0.0f; t < 1.0f; t += HERMITE_PRECISION) 
    {
        float phi0 = PHI0(t);
        float phi1 = PHI1(t);
        float psi0 = PSI0(t);
        float psi1 = PSI1(t);

        float px = p0x * phi0 + p1x * psi0 + v0x * phi1 + v1x * psi1;
        float py = p0y * phi0 + p1y * psi0 + v0y * phi1 + v1y * psi1;
        float pz = Vectormath::Aos::lerp(t, p0, p1).getZ();

        out_points[i++] = v4(px, py, pz, 1.0f);
    }

    return i;
}