#ifndef FCURVE_H
#define FCURVE_H

class fcurve {
enum ECurveType {
    F_CONSTANT,
    F_LINEAR,
    F_QUADRATIC,
    F_CUBIC,
    F_WAVE,
    F_BOX,
    F_SAW,
    F_MAX
};
public:
    v4 x;
    v4 y;
    v4 p;
    u32 type;
};

#endif // FCURVE_H