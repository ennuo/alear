#pragma once

class CThing;

void HandleAutoActivatedBouncepads(CThing*); 

namespace Launcher
{
    CThing* FindFromSurface(CThing*);
    float GetDistance(CThing*);
    void SetDistance(CThing*, float);
    float GetSpeed(CThing*);
    float GetAnimationFactor(CThing*);
    void SetAnimationFactor(CThing*, float);
    bool IsAutoActivated(CThing*);
    void SetAutoActivated(CThing*, bool);
    bool IsActive(CThing*);
    
    void Attach();
}

