#ifndef POPPET_INVENTORY_H
#define POPPET_INVENTORY_H

#include <vector.h>

#include <PoppetChild.h>
#include <hack_thingptr.h>
#include <Capture.h>

class RLevel;


class CPoppetInventory : public CPoppetChild {
public:
    void TakePlan(CVector<CThingPtr> const& things);
    void SaveMorphToInventory();
public:
    // 0x14 -> CVector<CThingPtr> CurrentSaveList
    // 0x20 -> CThingPtr CurrentSaveThing
    // 0x2c -> CP<RLevel> CurrentSaveLevel
    // 0x30 ->
    // 0x34 -> EType CurrentSaveType

    CVector<CThingPtr> CurrentSaveList;
    CThingPtr CurrentSaveThing;
    CP<RLevel> CurrentSaveLevel;
private:
    int Pad0;
public:
    NCapture::EType CurrentSaveType;
private:
    char Pad1[0x48];
public:
    v2 SelectBoxBounds; // 0x80;
};

#endif // POPPET_INVENTORY_H