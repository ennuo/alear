#ifndef PART_PHYSICS_WORLD_H
#define PART_PHYSICS_WORLD_H

#include "Part.h"
#include "vector.h"

class PPos;
class PRenderMesh;

class RLevel;
class PWorld : public CPart {
public:
    u32 ThingUIDCounter;
    CRawVector<CThing*> Things;
    RLevel* Level;
private:
    char Pad[0x48];
public:
    CRawVector<PRenderMesh*> ListPRenderMesh;
    CRawVector<PPos*> ListPPos;
public:
    CThing* GetThingByUID(u32 uid);
    CThing* FindThingByScriptName(char const* name);
};

#endif // PART_PHYSICS_WORLD_H