#include "SackBoyAnim.h"

#include "thing.h"
#include "RenderYellowHead.h"

#include <hook.h>

MH_DefineFunc(CSackBoyAnim_LoadAnims, 0x000f975c, TOC0, void, CSackBoyAnim*, CAnimBank*, bool);
void CSackBoyAnim::LoadAnims(CAnimBank* ab, bool cached)
{
    CSackBoyAnim_LoadAnims(this, ab, cached);
}

CRenderYellowHead* CSackBoyAnim::GetRenderYellowHead() const
{
    return Thing->GetPYellowHead()->GetRenderYellowHead();
}

void CSackBoyAnim::SetEffectMesh(RMesh* mesh)
{
    GetRenderYellowHead()->SetEffectMesh(mesh);
}

int CSackBoyAnim::GetNumFrames(int anim)
{
    return ScriptyStuff::GetNumFrames(Thing, anim);
}