#include "PartEmitter.h"

#include "thing.h"


#include <hook.h>

namespace NPoppetUtils {
    MH_DefineFunc(IsHeldByAnyPlayer, 0x003a566c, TOC1, bool, CThing const* thing, CThing const* player_ignore);
}

MH_DefineFunc(PEmitter_AttemptFire, 0x00077fb8, TOC0, CThing*, PEmitter*, bool, CThing*);
CThing* PEmitter::AttemptFire(bool predict, CThing* thing)
{
    return PEmitter_AttemptFire(this, predict, thing);
}

MH_DefineFunc(PEmitter_AttemptFireInternal, 0x000779f0, TOC0, CThing*, PEmitter*, bool, CThing*);
CThing* PEmitter::AttemptFireInternal(bool predict, CThing* thing)
{
    return PEmitter_AttemptFireInternal(this, predict, thing);
}

MH_DefineFunc(PEmitter_UpdateParent, 0x00060b38, TOC0, CThing*, PEmitter*);
void PEmitter::UpdateParent()
{
    PEmitter_UpdateParent(this);
}

MH_DefineFunc(PEmitter_CheckTooManyThingsEmitted, 0x0002fd38, TOC0, CThing*, PEmitter*);
void PEmitter::CheckTooManyThingsEmitted()
{
    PEmitter_CheckTooManyThingsEmitted(this);
}

CThing* PEmitter::Update(bool predict, unsigned int frame)
{
    const int& Behaviour = GetThing()->Behaviour;

    if (NPoppetUtils::IsHeldByAnyPlayer(GetThing(), NULL)) return NULL;
    
    if (!predict)
    {
        UpdateParent();
        CheckTooManyThingsEmitted();
    }

    if (Frequency == 0) return NULL;

    float current_frame = frame;
    float last_frame = frame - 1;

    CSwitchOutput* input = GetThing()->GetInput(0);
    if (input == NULL)
    {
        if (!predict)
        {
            ModDeltaFrames = 0.0f;
            ModStartFrame = 0.0f;
            ModScale = 1.0f;
        }
    }
    else
    {
        if (Behaviour == EMITTER_BEHAVIOR_ONE_SHOT) return NULL;

        float analogue;
        if (Behaviour == EMITTER_BEHAVIOR_SPEED_SCALE)
            analogue = input->Activation.Analogue;
        else if (Behaviour == EMITTER_BEHAVIOR_OFF_ON)
            analogue = input->Activation.Ternary == 0 ? 0.0f : 1.0f;
        
        float last_speed_scale = ModScale;
        float last_start_frame = ModStartFrame;
        float last_delta_frames = ModDeltaFrames;

        if (analogue != last_speed_scale && !predict)
        {
            ModStartFrame += (ModScale * (current_frame - ModDeltaFrames) - (current_frame - ModDeltaFrames));
            ModDeltaFrames = frame;
            ModScale = analogue;
        }

        if (analogue == 0.0f) return NULL;

        last_frame = (last_frame - last_delta_frames) * last_speed_scale + (last_start_frame + last_delta_frames);
        current_frame = (current_frame - ModDeltaFrames) * ModScale + (ModDeltaFrames + ModStartFrame);
    }

    current_frame += Phase;
    last_frame = floorf((last_frame + (float)Phase) / (float)Frequency);

    if (last_frame * (float)Frequency + (float)Frequency <= current_frame)
        return AttemptFireInternal(predict, NULL);

    return NULL;
}