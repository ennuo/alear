#ifndef OUTFITS_H
#define OUTFITS_H

#include "resources/ResourceOutfitList.h"

#include <refcount.h>

#include <ResourcePlan.h>
#include <thing.h>

bool LoadOutfits();
void InitOutfitHooks();

void CreateOutfitCollectBubble(CThing* player, CP<RPlan> const& plan);

#endif // OUTFITS_H