#include <PartSpriteLight.h>
#include <ResourceGame.h>
#include <thing.h>

const CGUID E_KEY_CANDLE_FLAME_MOL = 10539;

float PSpriteLight::GetOnDest() const
{
    CThing* t = GetThing()->GetParent();
    while (t != NULL)
    {
        if (t->ObjectType == OBJECT_BOUNCE_PAD)
            return t->GetPScript()->GetValue<float>("AnimationFactor", 0.0f);
        t = t->GetParent();
    }

    return OnDest;
}

void PSpriteLight::UpdateFlicker()
{
    if (FlickerProb > 0.0f && FlickerAmount > 0.0f)
    {
        float speed = GetOnDest() - OnCur;
        if (0.0f < speed && OnSpeed < speed) speed = OnSpeed;
        if (speed < 0.0f && OffSpeed < -speed)
            speed = -OffSpeed;
        OnCur += speed;
        
        if (FlickerProb < gGame->GetNonDeterministicRandStream().GetFloat())
        {
            if (OnCur < gGame->GetNonDeterministicRandStream().GetFloat())
                CurFlicker = 1.0f - FlickerAmount;
            else
                CurFlicker = 1.0f;
        }
    }
    else
    {
        OnCur = GetOnDest();
    }
}
