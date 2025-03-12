#ifndef RENDER_YELLOW_HEAD_H
#define RENDER_YELLOW_HEAD_H

#include <refcount.h>

#include <GuidHash.h>
#include "ResourceGFXMesh.h"
#include "ResourceSettingsSoftPhys.h"
#include "AnimBank.h"
#include "SackBoyAnim.h"
#include "MorphConstants.h"

class CThing;
class PRenderMesh;

class CRenderYellowHead {
public:
    void RemoveSackBoyAnim();
    void SnapshotCostume();
    void SetMesh(CP<RMesh> const& mesh);
    void SetEffectMesh(CP<RMesh> const& mesh);
    void SetSoftbodySim(bool enabled);
public:
    inline const CThing* GetYellowThing() const { return YellowThing; }
    inline const PRenderMesh* GetEffectMesh() const { return EffectMesh; }
public:
    m44 BodyOfs;
    m44 RibbonMat;
    m44 SmokeDummyOffset;
    m44 MeshMat;
    m44 EffectMat;
    m44 LastLimbMat[4];
    m44 LastHeadMat;
    v2 LocalGrabTargetDir;
    v2 GlobalGrabTargetDir;
    v2 GrabForce;
    v4 PelvisOffset;
    v4 PelvisOffsetOld;
    CMorphWeightInstance MorphWeightInstance;
    const CThing* YellowThing;
    const PRenderMesh* EffectMesh;
private:
    char Pad1[0xbc];
public:
    RSettingsSoftPhys* InstanceSoftPhys[2];
    float* BlendClusterRigidity[2];
private:
    char Pad2[0x34];
public:
    CSackBoyAnim* SackBoyAnim; // 0x458
};

extern bool gCachedAnimLoad;
extern int gCachedAnimIndex;

namespace ScriptyStuff {
    int LoadAnim(CAnimBank* ab, CGUID guid);
    int GetClusterCount(CThing* thing, int mesh);
    void SetBlendClusterRigidity(CThing* thing, int mesh, int cluster, float rigidity);
    void SetSoftPhysClusterEffect(CThing* thing, int mesh, int cluster, float effect);
    void RestoreMesh(CThing* thing);
    int GetNumFrames(CThing* thing, int anim);
    void SampleAnimi(CThing* thing, int dst, int anim, int frame, bool looped);
    void SetSoftPhysSpringScale(CThing* thing, int mesh, float scale);
};


#endif // RENDER_YELLOW_HEAD_H