#ifndef MISC_MESH_STYLES_H
#define MISC_MESH_STYLES_H

#include <GuidHash.h>

class CThing;

enum ELevelKeyStyle {
    LEVEL_KEY_STYLE_GEMS,
    LEVEL_KEY_STYLE_BRASS,
    LEVEL_KEY_STYLE_FOIL,
    LEVEL_KEY_STYLE_NULL,

    NUM_LEVEL_KEY_STYLES
};

CGUID GetMeshGUID(CThing* thing);

s32 GetLevelKeyType(CThing* thing);
s32 GetLevelKeyStyle(CThing* thing);

void SetLevelKeyStyle(CThing* thing, s32 style_index);


#endif // MISC_MESH_STYLES_H