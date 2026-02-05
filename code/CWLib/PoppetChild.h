#ifndef POPPET_CHILD_H
#define POPPET_CHILD_H

#include "hack_thingptr.h"

class CPoppet;
class PWorld;

class CPoppetChild {
public:
    CPoppetChild();
public:
    PWorld* GetWorld();
public:
    inline CPoppet* GetParent() const { return Parent; } 
    inline CThing* GetPlayer() const { return PlayerThing; } 
public:
    virtual ~CPoppetChild();
    virtual void SetParent(CPoppet* poppet);
    virtual void SetPlayer(CThing* player);
protected:
    CPoppet* Parent;
    CThingPtr PlayerThing;
};


#endif // POPPET_CHILD_H