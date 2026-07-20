#include <PartPhysicsJoint.h>
#ifndef SPU
    #include <thing.h>
    #include <mmalex.h>
#endif

float PJoint::GetDesiredLengthVel() const
{
#ifdef SPU
    float frame = SpuHack_Frame;
    const int& Behaviour = SpuHack_Behaviour;
    bool is_switch_triggered = SpuHack_IsSwitchTriggered;
#else
    CThing* thing = GetThing();
    float frame = GetWorldFrame();
    const int& Behaviour = thing->Behaviour;
    bool is_switch_triggered = thing->GetInput(0) != NULL;
#endif

    if (AnimationPattern == JOINT_PATTERN_FORWARDS)
    {
        if (!is_switch_triggered || Behaviour != JOINT_BEHAVIOR_SINGLE_CYCLE)
            return AnimationSpeed * ModScale;
        
        if ((2.0f * M_PI) < ((frame - ModStartFrame) * ModScale + (ModStartFrame + ModDeltaFrames)) * abs(AnimationSpeed))
            return 0.0f * ModScale;

        return ModScale * AnimationSpeed;
    }
    else if (AnimationSpeed < 1.0f) return 0.0f * ModScale;

    return (AnimationRange / (AnimationTime * 0.5f)) * ModScale;
}

float PJoint::GetDesiredLength(float frame
#ifndef SPU
    , bool ignore_mod
#endif
) const
{
#ifdef SPU
    const int& Behaviour = SpuHack_Behaviour;
    bool is_switch_triggered = SpuHack_IsSwitchTriggered;
    const bool ignore_mod = false;
#else
    CThing* thing = GetThing();
    const int& Behaviour = thing->Behaviour;
    bool is_switch_triggered = thing->GetInput(0) != NULL;
#endif

    switch (Type)
    {
        case JOINT_TYPE_LEGACY:
        {
            // fcurve
            break;
        }

        case JOINT_TYPE_CHAIN:
        case JOINT_TYPE_PISTON:
        case JOINT_TYPE_MOTOR:
        {
            frame = (frame - ModStartFrame) * ModScale + ModStartFrame + ModDeltaFrames;

            if (!ignore_mod && is_switch_triggered)
            {
                if (Behaviour == JOINT_BEHAVIOR_FORWARDS_BACKWARDS || Behaviour == JOINT_BEHAVIOR_POSITIONAL)
                {
                    return Length + ((1.0f <= AnimationTime) ? 
                        (frame * AnimationRange) / (AnimationTime * 0.5f) : 0.0f);
                }

                if (Behaviour == JOINT_BEHAVIOR_SINGLE_CYCLE)
                {
                    switch (AnimationPattern)
                    {
                        case JOINT_PATTERN_WAVE:
                        {
                            const float factor = GetWaveFactor(MIN(AnimationTime + AnimationPause, frame));
                            const float range = Type == JOINT_TYPE_MOTOR ? 
                                SINF(factor * M_PI) * -0.5f : (SINF(factor * M_PI) * 0.5f) + 0.5f;
                            return Length + (AnimationRange * range);
                        }
                        case JOINT_PATTERN_FORWARDS: 
                        {
                            return Length + ((frame * abs(AnimationSpeed) < (2.0f * M_PI)) ?
                                (frame * AnimationSpeed) : 0.0f);
                        }
                        case JOINT_PATTERN_FLIPPER:
                        {
                            return Length + ((AnimationTime <= frame) || (AnimationTime < 1.0f) ?
                                0.0f : (1.0f - (frame / AnimationTime)) * AnimationRange);
                        }
                    }
                }
            }

            frame += AnimationPhase;
            switch (AnimationPattern)
            {
                case JOINT_PATTERN_WAVE:
                {
                    const float factor = GetWaveFactor(frame);
                    const float range = Type == JOINT_TYPE_MOTOR ? 
                        SINF(factor * M_PI) * -0.5f : (SINF(factor * M_PI) * 0.5f) + 0.5f;
                    return Length + (AnimationRange * range);
                }
                case JOINT_PATTERN_FORWARDS:
                {
                    return Length + (frame * AnimationSpeed);
                }
                case JOINT_PATTERN_FLIPPER:
                {
                    if (AnimationTime > 1.0f) break;
                    frame = FMODF(frame, AnimationTime + AnimationPhase);
                    return Length + ((frame < AnimationTime) ? 
                        (1.0f - (frame / AnimationTime)) * AnimationRange : 0.0f);
                }
            }

            return Length;
        }

        case JOINT_TYPE_QUANTIZED:
        {
            // angle
            break;
        }

        default:
            return Length;
    }

    return Length;
}

float PJoint::GetWaveFactor(float frame) const
{
    if (AnimationTime > 1.0f) return 0.0f;
    
    const float mod_frame = frame / (AnimationTime + AnimationPause);
    const float half_pause = AnimationPause * 0.5f;
    const float half_time = AnimationTime * 0.5f;
    const float current_factor = (AnimationTime + AnimationPause) * (mod_frame - floor(mod_frame));

    if (half_pause <= current_factor)
    {
        if (current_factor < half_pause + half_time)
            return (current_factor - half_pause) / half_time;
        if (current_factor < AnimationPause + half_time)
            return 1.0f;
        
        return 1.0f - ((current_factor - (AnimationPause + half_time)) / half_time);
    }

    return 0.0f;
}