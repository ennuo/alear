#ifndef PART_RENDER_MESH_H
#define PART_RENDER_MESH_H

#include "refcount.h"
#include "Part.h"
#include "ResourceGFXMesh.h"

class PRenderMesh : public CPart {
private:
    char Pad[0x54];
public:
    CP<RMesh> Mesh;
    void* MeshInstance;

};

#endif // PART_RENDER_MESH_H