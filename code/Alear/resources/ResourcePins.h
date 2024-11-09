#ifndef RESOURCE_PINS_H
#define RESOURCE_PINS_H

#include <refcount.h>
#include <vector.h>

#include <Resource.h>
#include <ResourceGFXTexture.h>
#include <ResourceDescriptor.h>

class CPin {
public:
    u32 ID;
    u32 ProgressType;
    u32 Category;
    u32 TitleLamsKey;
    u32 DescriptionLamsKey;
    CResourceDescriptor<RTexture> Icon;
    u32 InitialProgressValue;
    u32 TargetValue;
    s8 TrophyToUnlock;
    u16 BehaviourFlags;
    s8 TrophyToUnlockLBP1;
};

class RPins : public CResource {
public:
    inline RPins(EResourceFlag flags) : CResource(flags, RTYPE_PINS), Pins() {}
public:
    u32 GetSizeInMemory() { return sizeof(RPins); }
public:
    CVector<CPin> Pins;
};

extern StaticCP<RPins> gPins;

#include "Serialise.h"
#include "Variable.h"
template <typename R>
ReflectReturn Reflect(R& r, CPin& d);
template <typename R>
ReflectReturn Reflect(R& r, RPins& d);

#endif // RESOURCE_PINS_H