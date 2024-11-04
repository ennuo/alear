#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H


#include "frustcull.h"


class CThing;


class __attribute__((aligned(0x80))) CMeshInstance {
public:
    CCullBone CullBone;
    v4 InstanceColor;
private:
    char Pad[0xe8];
public:
    CThing* MyThing;
private:
    char Pad2[0x84];
};

#endif // SCENE_GRAPH_H