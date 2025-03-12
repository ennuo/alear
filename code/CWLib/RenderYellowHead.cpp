#include "RenderYellowHead.h"
#include "ResourceSystem.h"
#include "ResourceGFXMesh.h"
#include "thing.h"

#include <hook.h>

MH_DefineFunc(CRenderYellowHead_RemoveSackBoyAnim, 0x000e5c34, TOC0, void, CRenderYellowHead*);
void CRenderYellowHead::RemoveSackBoyAnim()
{
    CRenderYellowHead_RemoveSackBoyAnim(this);
}

MH_DefineFunc(CRenderYellowHead_SnapshotCostume, 0x000e6ad0, TOC0, void, CRenderYellowHead*);
void CRenderYellowHead::SnapshotCostume()
{
    CRenderYellowHead_SnapshotCostume(this);
}

MH_DefineFunc(CRenderYellowHead_SetMesh, 0x000e4f08, TOC0, void, CRenderYellowHead*, CP<RMesh> const&);
void CRenderYellowHead::SetMesh(CP<RMesh> const& mesh)
{
    CRenderYellowHead_SetMesh(this, mesh);
}

MH_DefineFunc(CRenderYellowHead_SetEffectMesh, 0x000e5484, TOC0, void, CRenderYellowHead*, CP<RMesh> const&);
void CRenderYellowHead::SetEffectMesh(CP<RMesh> const& mesh)
{
    CRenderYellowHead_SetEffectMesh(this, mesh);
}

MH_DefineFunc(CRenderYellowHead_SetSoftbodySim, 0x000e4528, TOC0, void, CRenderYellowHead*, bool);
void CRenderYellowHead::SetSoftbodySim(bool enabled)
{
    CRenderYellowHead_SetSoftbodySim(this, enabled);
}

namespace ScriptyStuff 
{
    int LoadAnim(CAnimBank* ab, CGUID guid)
    {
        if (gCachedAnimLoad) return gCachedAnimIndex++;
    
        // the actual anim bank doesn't seem to get passed to this?
        ab = gAnimBank;
    
        if (ab->Subst) ab->Subst->Get(guid, guid);
    
        int anim_index = ab->Anim.size();
        CResourceDescriptor<RAnim> desc(guid);
        CP<RAnim> anim = LoadResource<RAnim>(desc, STREAM_PRIORITY_DEFAULT, 0, false);
        ab->Anim.push_back(anim);
        return anim_index;
    }

    int GetClusterCount(CThing* thing, int mesh)
    {
        CRenderYellowHead* rend = thing->GetPYellowHead()->GetRenderYellowHead();
        
        const PRenderMesh* pmesh = NULL;
        if (mesh == 0) pmesh = rend->GetYellowThing()->GetPRenderMesh();
        else if (mesh == 1) pmesh = rend->GetEffectMesh();
        else return 0;

        if (pmesh != NULL && pmesh->Mesh)
            return pmesh->Mesh->mesh.SoftbodyCluster.ClusterCount;
        
        return 0;
    }

    void SetBlendClusterRigidity(CThing* thing, int mesh, int cluster, float rigidity)
    {
        thing->GetPYellowHead()->GetRenderYellowHead()->BlendClusterRigidity[mesh][cluster] = rigidity;
    }

    void SetSoftPhysClusterEffect(CThing* thing, int mesh, int cluster, float effect)
    {
        thing->GetPYellowHead()->GetRenderYellowHead()->InstanceSoftPhys[mesh]->ClusterEffect[cluster] = effect;
    }

    MH_DefineFunc(_RestoreMesh, 0x000e63d4, TOC0, void, CThing*);
    void RestoreMesh(CThing* thing)
    {
        _RestoreMesh(thing);
    }

    int GetNumFrames(CThing* thing, int anim)
    {
        return gAnimBank->Anim[anim]->NumFrames;
    }

    MH_DefineFunc(_SampleAnimi, 0x000e432c, TOC0, void, CThing*, int, int, int, bool);
    void SampleAnimi(CThing* thing, int dst, int anim, int frame, bool looped)
    {
        _SampleAnimi(thing, dst, anim, frame, looped);
    }

    void SetSoftPhysSpringScale(CThing* thing, int mesh, float scale)
    {
        thing->GetPYellowHead()->GetRenderYellowHead()->InstanceSoftPhys[mesh]->SpringScale = scale;
    }
}