#include "resources/ResourcePins.h"
#include <Serialise.h>

StaticCP<RPins> gPins;

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;

template <typename R>
ReflectReturn Reflect(R& r, CPin& d)
{
    ReflectReturn ret;
    ADD(ID);
    ADD(ProgressType);
    ADD(Category);
    ADD(TitleLamsKey);
    ADD(DescriptionLamsKey);
    ADD(Icon);
    ADD(InitialProgressValue)
    ADD(TargetValue);
    ADD(TrophyToUnlock);
    ADD(BehaviourFlags);
    // ADD(TrophyToUnlockLBP1);
    return ret;
}

template <typename R>
ReflectReturn Reflect(R& r, RPins& d)
{
    ReflectReturn ret;
    ADD(Pins);
    return ret;
}

template <typename R>
ReflectReturn Reflect(R& r, CPinsAwarded& d)
{
    ReflectReturn ret;
    ADD(PinAwards);
    ADD(PinProgress);
    ADD(RecentlyAwardedPinIDs);
    ADD(ProfileDisplayPinIDs);
    ADD(PinsFlags);
    return ret;
}

template <typename R>
ReflectReturn Reflect(R& r, CPinAward& d)
{
    ReflectReturn ret;
    ADD(PinID);
    ADD(AwardCount);
    return ret;
}

template <typename R>
ReflectReturn Reflect(R& r, CPinProgress& d)
{
    ReflectReturn ret;
    ADD(ProgressType);
    ADD(ProgressCount);
    return ret;
}

template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, CPin& d);
template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, RPins& d);

template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, CPinsAwarded& d);
template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, CPinProgress& d);
template ReflectReturn Reflect<CReflectionLoadVector>(CReflectionLoadVector& r, CPinAward& d);

template ReflectReturn Reflect<CReflectionSaveVector>(CReflectionSaveVector& r, CPinsAwarded& d);
template ReflectReturn Reflect<CReflectionSaveVector>(CReflectionSaveVector& r, CPinProgress& d);
template ReflectReturn Reflect<CReflectionSaveVector>(CReflectionSaveVector& r, CPinAward& d);

#undef ADD