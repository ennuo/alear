#ifndef OUTFITS_H
#define OUTFITS_H

#include "resources/ResourceOutfitList.h"

#include <refcount.h>

#include <ResourcePlan.h>
#include <GuidHash.h>
#include <thing.h>

bool LoadOutfits();
void InitOutfitHooks();

void DoOutfitCheck(CThing* player, CGUID guid);
void SpawnCollectBubble(CThing* player, CP<RPlan> const& plan);

#endif // OUTFITS_H