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

MH_DefineFunc(PJoint_GetWaveFactor, 0x000284bc, TOC0, float, const PJoint*, float);
float PJoint::GetWaveFactor(float f) const
{
    return PJoint_GetWaveFactor(this, f);
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

float PJoint::GetMaxDesiredVel() const
{
    if (Type != JOINT_TYPE_MOTOR && Type != JOINT_TYPE_PISTON && Type != JOINT_TYPE_CHAIN)
        return 0.0f;

    const int& Behaviour = GetThing()->Behaviour;
    bool is_switch_triggered = GetThing()->GetInput(0) != NULL;

    if (is_switch_triggered && (Behaviour == JOINT_BEHAVIOR_FORWARDS_BACKWARDS || Behaviour == JOINT_BEHAVIOR_POSITIONAL))
    {
        return AnimationTime < 1.0f ? 0.0f : abs((AnimationRange / AnimationTime) * 2.0f);
    }

    if (AnimationPattern == JOINT_PATTERN_FLIPPER) return AnimationRange;
    if (AnimationPattern == JOINT_PATTERN_FORWARDS) return abs(AnimationSpeed);
    if (AnimationPattern != JOINT_PATTERN_WAVE) return 0.0f;

    return AnimationTime < 1.0f ? 0.0f : abs((AnimationRange / AnimationTime) * 3.1415f);

}

float PJoint::GetDesiredLengthVel() const
{
    // supposed to be passed in, but it'll usually just be the level's
    // forked frame
    float frame = A ? A->World->GetFrame() : 0.0f;
    const int& Behaviour = GetThing()->Behaviour;
    bool is_switch_triggered = GetThing()->GetInput(0) != NULL;

    if (AnimationPattern == JOINT_PATTERN_FORWARDS)
    {
        if (is_switch_triggered || Behaviour != JOINT_BEHAVIOR_SINGLE_CYCLE)
            return AnimationSpeed * ModScale;

        if (6.2831855f < ((frame - ModStartFrame) * ModScale + (ModStartFrame + ModDeltaFrames)) * abs(AnimationSpeed))
            return 0.0f * ModScale;

        return ModScale * AnimationSpeed;
    }
    else if (AnimationSpeed < 1.0f) return 0.0f * ModScale;
    
    return (AnimationRange / (AnimationTime * 0.5f)) * ModScale;
}

float PJoint::GetDesiredLength(float frame, bool ignore_mod) const
{
    const int& Behaviour = GetThing()->Behaviour;
    bool is_switch_triggered = GetThing()->GetInput(0) != NULL;

    switch (Type)
    {
        case JOINT_TYPE_LEGACY:
        {
            // fcurve
            break;
        }

        case JOINT_TYPE_ELASTIC:
        case JOINT_TYPE_SPRING:
        case JOINT_TYPE_STRING:
        case JOINT_TYPE_ROD:
        case JOINT_TYPE_BOLT:
        case JOINT_TYPE_SPRING_ANGULAR:
            return Length;

        case JOINT_TYPE_CHAIN:
        case JOINT_TYPE_PISTON:
        case JOINT_TYPE_MOTOR:
        {
            frame = (frame - ModStartFrame) * ModScale + ModStartFrame + ModDeltaFrames;
            float range = 0.0f;

            if (ignore_mod || is_switch_triggered || Behaviour == JOINT_BEHAVIOR_ON_OFF || Behaviour == JOINT_BEHAVIOR_SPEED_SCALE)
            {
                frame += AnimationPhase;
                if (AnimationPattern == JOINT_PATTERN_FORWARDS)
                {
                    range = frame * AnimationSpeed;
                    return (float)Length + range;
                }
                else if (AnimationPattern == JOINT_PATTERN_FLIPPER)
                {
                    if (1.0f <= AnimationTime)
                    {
                        frame = mmalex::fmod(frame, AnimationTime + AnimationPause);
                        if (frame < AnimationTime)
                            range = (1.0f - (frame / AnimationTime)) * AnimationRange;
                    }

                    return (float)Length + range;
                }

            }
            else
            {
                if (Behaviour == JOINT_BEHAVIOR_FORWARDS_BACKWARDS || Behaviour == JOINT_BEHAVIOR_POSITIONAL)
                {
                    if (1.0f <= AnimationTime)
                        range = (frame * AnimationRange) / (AnimationTime * 0.5f);
                    return (float)Length + range;
                }
                else if (Behaviour == JOINT_BEHAVIOR_SINGLE_CYCLE)
                {
                    if (AnimationPattern == JOINT_PATTERN_FORWARDS)
                    {
                        if (frame * abs(AnimationSpeed) < 6.2831855f)
                            range = frame * AnimationSpeed;
                        return (float)Length + range;
                    }
                    else if (AnimationPattern == JOINT_PATTERN_FLIPPER)
                    {
                        if (frame < AnimationTime && 1.0f <= AnimationTime)
                            range = (1.0f - (frame / AnimationTime)) * AnimationRange;
                        return (float)Length + range;
                    }
                    else frame = MIN(AnimationTime + AnimationPause, frame);
                }
            }

            float factor = GetWaveFactor(frame);
            if (Type == JOINT_TYPE_MOTOR)
                range = mmalex::sin(factor * M_PI) * -0.5f;
            else
                range = mmalex::sin(factor * M_PI) * -0.5f + 0.5f;

            range *= AnimationRange;

            return range + Length;
        }

        case JOINT_TYPE_QUANTIZED:
        {
            // angle
            break;
        }
    }
}