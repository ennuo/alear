#ifndef SPRING_H
#define SPRING_H

#include <vector.h>

struct SoftbodySpring {
    u16 a;
    u16 b;
    float rest_length_sq;
};

typedef CRawVector<SoftbodySpring, CAllocatorMMAligned128> SoftbodySpringVec;

#endif // SPRING_H