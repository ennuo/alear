#include "PoppetBubble.h"
#include <hook.h>

MH_DefineFunc(CPoppetBubble_SetBubblePolygon, 0x00358730, TOC1, void, CPoppetBubble*, const CRawVector<v2, CAllocatorMMAligned128>&);
MH_DefineFunc(CPoppetBubble_SetBubbleSticker, 0x0035a9b0, TOC1, void, CPoppetBubble*);
MH_DefineFunc(CPoppetBubble_SetBubbleShape, 0x00358ac0, TOC1, void, CPoppetBubble*, PShape*);
MH_DefineFunc(CPoppetBubble_IsTweakBubble, 0x00358040, TOC1, bool, CPoppetBubble*);
MH_DefineFunc(CPoppetBubble_SetBubbleRect, 0x0035a778, TOC1, void, CPoppetBubble*, float, float);
MH_DefineFunc(CPoppetBubble_SetBubbleRoundedRect, 0x0035a6ec, TOC1, void, CPoppetBubble*, float, float);
MH_DefineFunc(CPoppetBubble_SetBubbleBalloon, 0x0035a188, TOC1, void, CPoppetBubble*, float, float, float);
MH_DefineFunc(CPoppetBubble_SetBubbleCircle, 0x0035a120, TOC1, void, CPoppetBubble*, float);
MH_DefineFunc(CPoppetBubble_SetBubbleObject, 0x00358d40, TOC1, bool, CPoppetBubble*, CThing*, bool);
MH_DefineFunc(CPoppetBubble_CalcBubbleMatrix, 0x00359958, TOC1, m44, CPoppetBubble*);

bool CPoppetBubble::IsTweakBubble()
{
    return CPoppetBubble_IsTweakBubble(this);
}

void CPoppetBubble::SetBubbleSticker()
{
    CPoppetBubble_SetBubbleSticker(this);
}

void CPoppetBubble::SetBubbleRect(float width, float height)
{
    CPoppetBubble_SetBubbleRect(this, width, height);
}

void CPoppetBubble::SetBubbleRoundedRect(float width, float height)
{
    CPoppetBubble_SetBubbleRoundedRect(this, width, height);
}

void CPoppetBubble::SetBubbleBalloon(float width, float height, float egginess)
{
    CPoppetBubble_SetBubbleBalloon(this, width, height, egginess);
}

void CPoppetBubble::SetBubbleShape(PShape* shape)
{
    CPoppetBubble_SetBubbleShape(this, shape);
}

void CPoppetBubble::SetBubblePolygon(const CRawVector<v2, CAllocatorMMAligned128>& poly)
{
    CPoppetBubble_SetBubblePolygon(this, poly);
}

void CPoppetBubble::SetBubbleCircle(float radius)
{
    CPoppetBubble_SetBubbleCircle(this, radius);
}

bool CPoppetBubble::SetBubbleObject(CThing* thing, bool centre)
{
    return CPoppetBubble_SetBubbleObject(this, thing, centre);
}

MH_DefineFunc(CPopeptBubble_SetBubbleModeID, 0x003586dc, TOC1, void, CPoppetBubble*, u32);
void CPoppetBubble::SetBubbleModeID(u32 id)
{
    CPopeptBubble_SetBubbleModeID(this, id);
    // if (BubbleModeID != id) TheBubble.SetWibbly();
    // BubbleModeID = id;
}

bool CPoppetBubble::FindBoundaryPosition(v2 direction, v2& pos_out) const
{
    return TheBubble.FindBoundaryPosition(direction, pos_out);
}

m44 CPoppetBubble::CalcBubbleMatrix()
{
    return CPoppetBubble_CalcBubbleMatrix(this);
}

// void CPoppetBubble::UpdateShape(bool in_use)
// {

// }