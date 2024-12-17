#ifndef HASH_H
#define HASH_H


extern u32 (*JenkinsHash)(u8* k, u32 length, u32 initial_value);
// u32 JenkinsHash(u8* k, u32 length, u32 initial_value);

#endif // HASH_H