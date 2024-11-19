#include "PartGameplayData.h"

void PGameplayData::SetEggLink(CP<RPlan> const& plan)
{
    if (EggLink == NULL) EggLink = new CEggLink();
    EggLink->Set(plan);
}