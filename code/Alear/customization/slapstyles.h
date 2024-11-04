#ifndef CUSTOMIZATION_SLAP_STYLES_H
#define CUSTOMIZATION_SLAP_STYLES_H

#include <mem_stl_buckets.h>
#include <MMString.h>

#include <thing.h>
#include <vector.h>
#include <GuidHash.h>

class CSlapMesh {
public:
    inline CSlapMesh() : 
    Sound(), GUID(0), Strength(1.0f), 
    LeftHand(false), RightHand(true), Kill(false)
    {

    }
public:
    MMString<char> Sound;
    u32 GUID;
    float Strength;
    bool LeftHand;
    bool RightHand;
    bool Kill;
};

struct CSlapStyles {
    CVector<CSlapMesh> Meshes;
};

extern CVector<CSlapMesh> gSlapMeshes;
void OnPlayerSlapped(CThing* victim, CThing* by, v2 force, v2 const* pos3d, bool side);

#endif // CUSTOMIZATION_SLAP_STYLES_H
