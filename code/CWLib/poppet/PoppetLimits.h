#ifndef POPPET_LIMITS_H
#define POPPET_LIMITS_H

#include <vector.h>

class CBaseLimit {
public:
    virtual ~CBaseLimit();
    virtual u32 GetMaxCount();
protected:
    void* Settings;
    bool Dirty;
private:
    int Type;
    u32 CurrentAmount;
};

class CLimitsHaver {
public:
    virtual ~CLimitsHaver();
protected:
    CRawVector<CBaseLimit*> Limits;
    float LastMaxLimit;
};

class CInventoryLimitsHaver : public CLimitsHaver {

};


#endif // POPPET_LIMITS_H