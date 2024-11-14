#ifndef PART_RENDER_MESH_H
#define PART_RENDER_MESH_H

#include <vector.h>
#include <refcount.h>

#include "Part.h"
#include "ResourceGFXMesh.h"

#include "hack_thingptr.h"

class PRenderMesh : public CPart {
private:
    //char Pad[0x54];
    char Pad[0x48];
public:
    CVector<CThingPtr> BoneThings;
    CP<RMesh> Mesh;
    void* MeshInstance;
private:
    char Pad2[0x12];
public:
    bool Visible;
};

#endif // PART_RENDER_MESH_H