#ifndef RAYCAST_H
#define RAYCAST_H

class CMeshInstance;
struct RaycastJobResult {
    v4 normal;
    const CMeshInstance* meshinstance;
    float t;
    u32 triindex;
    float ucoord;
    float vcoord;
    float baryucoord;
    float baryvcoord;
    u32 decorationindex;
    bool windingOrder;
    bool hit;
};

#endif // RAYCAST_H