#ifndef DECORATION_INFO_H
#define DECORATION_INFO_H

#include <refcount.h>

#include "hack_thingptr.h"
#include "ResourceDescriptor.h"

class CDecorationInfo {
public:
    float Angle;
    float Scale;
    u32 LastDecoration;
    CThingPtr LastDecoratedThing;
    CP<RMesh> Decoration;
    bool Reversed;
    bool Stamping;
    CResourceDescriptor<RPlan> RefPlan;
};

#endif // DECORATION_INFO_H