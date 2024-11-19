#ifndef EGG_LINK_H
#define EGG_LINK_H

#include <refcount.h>
#include <vector.h>
#include <ReflectionVisitable.h>

#include "ResourcePlan.h"

class CEggLink : public CReflectionVisitable {
public:
    inline CEggLink() : Plan(), Shareable(true), AutoPickupList() {}
    inline CEggLink(CP<RPlan>& plan) : Plan(plan), Shareable(true) {}

    inline CP<RPlan>& GetPlan() { return Plan; }
    inline bool IsValid() { return Plan.GetRef() != NULL; }
    inline void Set(CP<RPlan> const& plan) { Plan = plan; }
    inline void SetShareable(bool shareable) { Shareable = shareable; }
    inline bool GetShareable() { return Shareable; }
    inline void AddAutoPickup(RPlan* plan) 
    {
        CP<RPlan> cp(plan);
        AutoPickupList.push_back(cp);
    }

    inline CVector<CP<RPlan> >& GetAutoPickupList() { return AutoPickupList; }
private:
    CP<RPlan> Plan;
    bool Shareable;
    CVector<CP<RPlan> > AutoPickupList;
};


#endif // EGG_LINK_H