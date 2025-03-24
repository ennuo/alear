#include "AnimationSystem.h"

#include <ResourceGFXAnim.h>

#include <thing.h>


/* 
 * Re-implementation of animation sampling functions so we can
 * use our animation bank that's attached to the current CSackBoyAnim
 * instance rather than just the global one.
 * 
 * Allows for finer control over character animations, especially in multiplayer
 * situations.
*/

void SampleAnim(RPSAnimData& data, const RAnim* anim, int frame, bool looped)
{
    // anim->GetAnimDataForFrame(data.Rot.begin(), data.Pos.begin(), data.Scale.begin(), (v4*)data.Morph.begin(), frame, anim->GetMorphCount(), looped);
}

CAnimBank* GetAnimBank(const CThing* thing)
{
    return thing->GetPYellowHead()->GetRenderYellowHead()->AnimBank;    
}

int GetNumFrames(const CThing* thing, int anim)
{
    return GetAnimBank(thing)->Anim[anim]->NumFrames;
}

void SampleAnimi(const CThing* thing, int dst, int anim, int frame, bool looped)
{
    CP<RAnim>& res = GetAnimBank(thing)->Anim[anim];
    CRenderYellowHead* render = thing->GetPYellowHead()->GetRenderYellowHead();
    SampleAnim(*render->CurAnimTemp[dst], res, frame, looped);
}