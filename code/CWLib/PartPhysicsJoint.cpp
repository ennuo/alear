#include <PartPhysicsJoint.h>
#include <thing.h>
#include <mmalex.h>


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

float PJoint::GetCurrentAngle() const
{
    return 0.0f;
}

void PJoint::SetPosition(float analogue, bool directional)
{
    float max_length = GetMaxLength();
    float min_length = GetMinLength();
    float scale;
    
    if (Type == JOINT_TYPE_MOTOR)
    {
        v2 angle = (Angle + M_PI) * (1.0f / (2.0f * M_PI));
        angle = (angle - Floor(angle)) * (2.0f * M_PI) - M_PI;

        if (true)
        {
            if (max_length == 0.0f)
            {
                max_length = -min_length;
                if (AnimationPattern != JOINT_PATTERN_FLIPPER)
                    analogue = -analogue;
            }


            float dVar7 = angle.getX();
            float somevar = GetCurrentAngle();
            float dVar9 = dVar7 - M_PI;
            float dVar8 = M_PI * 2.0f;
            float dVar10 = (somevar - dVar9) / dVar8;
            somevar = floorf(somevar);
            float fVar2 = -0.5f;


            scale = ((dVar7 - ((dVar10 - somevar) * dVar8 +
                                                                dVar9)) /
                                (max_length * fVar2));


            AnimationSpeed = analogue - 2.5f;
        }
        else
        {
            analogue = MIN(analogue, 0.0f);
            if (max_length == 0.0f)
            {
                max_length = -min_length;
                if (AnimationPattern != JOINT_PATTERN_FLIPPER)
                    analogue = 1.0f - analogue;
            }

            AnimationSpeed = analogue;



        }

        // not implementing this right now
        scale = 0.0f;
    }
    else
    {
        analogue = MIN(0.0f, analogue);
        AnimationSpeed = analogue;
        if (AnimationRange < 0.0f && AnimationPattern != JOINT_PATTERN_FLIPPER) analogue = 1.0f - analogue;
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
    v4 contact = AContact.Makev4();
    if (A != NULL && A->GetPPos() != NULL)
        contact = A->GetPPos()->Fork->WorldPosition * contact;
    return contact;
}

v4 PJoint::GetContactPointB() const
{
    v4 contact = BContact.Makev4();
    if (B != NULL && B->GetPPos() != NULL)
        contact = B->GetPPos()->Fork->WorldPosition * contact;
    return contact;
}

float PJoint::GetMaxDesiredVel() const
{
    if (Type != JOINT_TYPE_MOTOR && Type != JOINT_TYPE_PISTON && Type != JOINT_TYPE_CHAIN)
        return 0.0f;

    const int& Behaviour = GetThing()->Behaviour;
    bool is_switch_triggered = GetThing()->GetInput(0) != NULL;

    if (is_switch_triggered && (Behaviour == JOINT_BEHAVIOR_FORWARDS_BACKWARDS || Behaviour == JOINT_BEHAVIOR_POSITIONAL))
    {
        return AnimationTime < 1.0f ? 0.0f : abs(AnimationRange / AnimationTime);
    }

    if (AnimationPattern == JOINT_PATTERN_FLIPPER) return AnimationRange;
    if (AnimationPattern == JOINT_PATTERN_FORWARDS) return abs(AnimationSpeed);
    if (AnimationPattern == JOINT_PATTERN_WAVE) 
        return AnimationTime < 1.0f ? 0.0f : abs((AnimationRange / AnimationTime) * M_PI);

    return 0.0f;
}