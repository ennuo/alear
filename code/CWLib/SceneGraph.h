#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H


#include "frustcull.h"
#include "thing.h"

class CMeshInstance {
public:
    CCullBone CullBone;
    v4 InstanceColor;
    char Pad[0xe8];
    CThing* MyThing;
};

#endif // SCENE_GRAPH_H