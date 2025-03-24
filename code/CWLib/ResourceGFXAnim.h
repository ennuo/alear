#ifndef RESOURCE_GFX_ANIM_H
#define RESOURCE_GFX_ANIM_H

#include <refcount.h>
#include <vector.h>

#include <Resource.h>


class RPSAnimData {
public:
    CRawVector<q4, CAllocatorMMAligned128> Rot;
    CRawVector<v4, CAllocatorMMAligned128> Pos;
    CRawVector<v4, CAllocatorMMAligned128> Scale;
    CRawVector<float, CAllocatorMMAligned128> Morph;
};

class RAnim : public CResource {
public:
    void GetAnimDataForFrame(m44* out, int frame, const unsigned char* flip_type, bool looped, bool parent) const;
    void GetAnimDataForFrame(m44* out, float* morph, unsigned int dst_morph_count, int frame, const unsigned char* flip_type, bool looped, bool parent) const;
    void GetAnimDataForFrame(q4* r_out, v4* p_out, v4* s_out, v4* m_out, int frame, unsigned int dst_morph_count, bool looped) const;
public:
    inline u32 GetMorphCount() const { return MorphCount; }
public:
    u32 NumFrames;
    u32 FPS;
    u32 LoopStart;
    u32 MorphCount;
};


#endif // RESOURCE_GFX_ANIM_H