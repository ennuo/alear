#ifndef POWERUP_H
#define POWERUP_H

#include "thing.h"
#include "PartCreature.h"
#include "cell/DebugLog.h"

void AlearInitCreatureHook();
void SetJetpackTether(CThing* thing, CThing* creature_thing, float tether_length, v2* param_4);
void CollectGun(CThing* thing, CThing* creature_thing);
void SetScubaGear(CThing* thing, bool active);

#endif // POWERUP_H