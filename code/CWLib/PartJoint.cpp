#include "PartJoint.h"
#include "hook.h"

MH_DefineFunc(PJoint_SetA, 0x000608ec, TOC1, void, PJoint* joint, CThing* thing);
void PJoint::SetA(PJoint* joint, CThing* thing)
{
    PJoint_SetA(PJoint* joint, CThing* thing);
}

MH_DefineFunc(PJoint_SetB, 0x0005ff0c, TOC1, void, PJoint* joint, CThing* thing);
void PJoint::SetB(PJoint* joint, CThing* thing)
{
    PJoint_SetB(PJoint* joint, CThing* thing);
}