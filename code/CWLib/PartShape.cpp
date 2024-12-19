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