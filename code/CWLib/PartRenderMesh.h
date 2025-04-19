#ifndef PART_RENDER_MESH_H
#define PART_RENDER_MESH_H

#include <vector.h>
#include <refcount.h>

#include "ResourceDescriptor.h"
#include "Part.h"
#include "ResourceGFXMesh.h"

#include "hack_thingptr.h"

class PRenderMesh : public CPart {
public:
    u16 HandyIndex;
    u16 FrustVisible;
    v4 EditorColour;
    v4 EditorColourTint;
    CP<RAnim> Anim;
    float AnimPos;
    float AnimPosOld;
    float AnimSpeed;
    float LoopStart;
    float LoopEnd;
    CThingPtr SpriteLightThing;
    CVector<CThingPtr> BoneThings;
    CP<RMesh> Mesh;
    void* MeshInstance;
private:
    char Pad2[0x12];
public:
    bool Visible;
};

#endif // PART_RENDER_MESH_H