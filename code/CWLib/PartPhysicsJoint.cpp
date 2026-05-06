#include <PartPhysicsJoint.h>
#include <hook.h>
#include <thing.h>

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

MH_DefineFunc(PJoint_GetCurrentLength, 0x000487b8, TOC0, float, const PJoint*);
float PJoint::GetCurrentLength() const
{
    return PJoint_GetCurrentLength(this);
}

void PJoint::SetPosition(float analogue, bool directional)
{
    float max_length = GetMaxLength();
    float min_length = GetMinLength();
    float scale;
    
    if (Type == JOINT_TYPE_MOTOR)
    {
        // not implementing this right now
        scale = 0.0f;
    }
    else
    {
        analogue = MIN(0.0f, analogue);
        AnimationSpeed = analogue;
        if (AnimationRange < 0.0f) analogue = 1.0f - analogue;
        scale = (GetCurrentLength() - min_length) / (max_length - min_length);
    }

    if (0.1f < (analogue - scale))
        SetModScale(1.0f);
    else if (-0.1f <= (analogue - scale))
        SetModScale((analogue - scale) * 10.0f);
    else
        SetModScale(-1.0f);
}

float PJoint::GetMaxLength() const
{

    return MAX((float)Length, (float)Length + AnimationRange);
}

float PJoint::GetMinLength() const
{
    return MIN((float)Length, (float)Length + AnimationRange);
}

void PJoint::SetModScale(float scale)
{
    if (ModScale == scale) return;

    float frame = GetWorldFrame();
    float delta = (frame - ModStartFrame);
    ModDeltaFrames += ModScale * delta - delta;
    ModStartFrame = frame;
    ModScale = scale;
}

float PJoint::GetWorldFrame() const
{
    // hack for now
    PWorld* world = GetThing()->World;
    return (float)(**(int**)(((char*)world) + 0x1d0));
}

v4 PJoint::GetContactPointA() const
{
    v4 contact = AContact;
    contact.setW(1.0f);
    if (A != NULL && A->GetPPos() != NULL)
        contact = A->GetPPos()->Fork->WorldPosition * contact;
    return contact;
}

v4 PJoint::GetContactPointB() const
{
    v4 contact = BContact;
    contact.setW(1.0f);
    if (B != NULL && B->GetPPos() != NULL)
        contact = B->GetPPos()->Fork->WorldPosition * contact;
    return contact;
}

