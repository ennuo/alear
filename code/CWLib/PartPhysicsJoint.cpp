#include "PartPhysicsJoint.h"
#include <hook.h>

MH_DefineFunc(PJoint_SetA, 0x000608ec, TOC1, void, PJoint*, CThing*);
void PJoint::SetA(CThing* thing)
{
    PJoint_SetA(this, thing);
}

MH_DefineFunc(PJoint_SetB, 0x0005ff0c, TOC1, void, PJoint*, CThing*);
void PJoint::SetB(CThing* thing)
{
    PJoint_SetA(this, thing);
}