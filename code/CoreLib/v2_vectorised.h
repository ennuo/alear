#ifndef V2_VECTORISED_H
#define V2_VECTORISED_H

struct vint {
    unsigned int V  __attribute__ ((__vector_size__ (16)));
};

struct vfloat {
    float V __attribute__ ((__vector_size__ (16)));
};

struct v2 {
    float V __attribute__ ((__vector_size__ (16)));
};

#endif // V2_VECTORISED_H