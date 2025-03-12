#include "PartShape.h"
#include <hook.h>

MH_DefineFunc(PShape_SetCollidableGame, 0x0002f0f0, TOC0, void, PShape*, bool);
void PShape::SetCollidableGame(bool collidable)
{
    PShape_SetCollidableGame(this, collidable);
}

MH_DefineFunc(PShape_SetCollidablePoppet, 0x000366c0, TOC0, void, PShape*, bool);
void PShape::SetCollidablePoppet(bool collidable)
{
    PShape_SetCollidablePoppet(this, collidable);
}

MH_DefineFunc(PShape_SetMaterial, 0x00056a5c, TOC0, void, PShape*, RMaterial*);
void PShape::SetMaterial(RMaterial* material)
{
    PShape_SetMaterial(this, material);
}

MH_DefineFunc(PShape_InitialisePolygon, 0x0005c7e0, TOC0, void, PShape*);
void PShape::InitialisePolygon()
{
    PShape_InitialisePolygon(this);
}

MH_DefineFunc(PShape_SetPolygon, 0x0005d16c, TOC0, void, PShape*, CRawVector<v2, CAllocatorMMAligned128> const&, CRawVector<unsigned int> const&);
void PShape::SetPolygon(CRawVector<v2, CAllocatorMMAligned128> const& vertices, CRawVector<unsigned int> const& indices)
{
    PShape_SetPolygon(this, vertices, indices);
}