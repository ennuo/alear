#ifndef SHA1_H
#define SHA1_H

enum ESHA1Result {
    SHA1_SUCCESS,
    SHA1_NULL,
    SHA1_INPUT_TOO_LONG,
    SHA1_STATE_ERROR
};

struct CSHA1Context { // sha1.h: 55
    int Computed;
    ESHA1Result Corrupted;
    struct 
    {
        u32 h0;
        u32 h1;
        u32 h2;
        u32 h3;
        u32 h4;
        u32 nblocks;
        unsigned char buf[64];
        int count;
    } Context;
};

#endif // SHA1_H