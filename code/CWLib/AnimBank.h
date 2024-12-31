#ifndef ANIM_BANK_H
#define ANIM_BANK_H

#include <refcount.h>
#include <vector.h>

#include <Resource.h>
#include <ResourceGuidSubst.h>

class RAnim : public CResource {};
class CAnimBank {
public:
    inline CAnimBank()
    {
        memset(this, 0, sizeof(CAnimBank));
        HeadBoneParentIndex = -1;
        HeadBoneIndex = -1;
    }
public:
    void InitPostResource();
public:
    u32 BoneCount;
    // u32 MeshGUID; // not added until deploy
    u16* MirrorBones;
    u16* MirrorMorphs;
    u8* MirrorBoneFlipType;
    CRawVector<u16> MirrorBoneVec;
    CRawVector<u16> MirrorMorphVec;
    CRawVector<u8> MirrorBoneFlipTypeVec;
    CRawVector<int> ParentBones;
    u32 HeadBoneParentIndex;
    u32 HeadBoneIndex;
    u32 GunPoseBoneIndex;
    u32 GunPoseParentBoneIndex;
    CVector<CP<RAnim> > Anim;
private:
    char Pad2[0x348];
public:
    CP<RGuidSubst> Subst; // This field exists in deploy, doesn't in retail I'm fairly sure, but I'm going to be using it since I overwrote the methods where it gets allocated anyway
};

extern CAnimBank* gAnimBank;

#endif // ANIM_BANK_H