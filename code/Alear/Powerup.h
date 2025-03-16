#ifndef POWERUP_H
#define POWERUP_H

#include "thing.h"
#include "PartCreature.h"
#include "cell/DebugLog.h"

void AlearInitCreatureHook();
void SetJetpackTether(CThing* player, CThing* attachment, float length, v2 pos);
void CollectGun(CThing* thing, CThing* creature_thing);
void StartGunState(CThing* creature_thing);
void SetScubaGear(CThing* thing, bool active);

#endif // POWERUP_H