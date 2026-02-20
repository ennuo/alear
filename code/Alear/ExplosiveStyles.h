#ifndef EXPLOSIVE_STYLES_H
#define EXPLOSIVE_STYLES_H

#include <GuidHash.h>

class CThing;

enum EExplosiveType {
    EXPLOSIVE_TNT,
    EXPLOSIVE_BANGER,

    NUM_EXPLOSIVE_TYPES
};

enum EExplosiveStyle {
    EXPLOSIVE_STYLE_STANDARD,
    EXPLOSIVE_STYLE_SHOCK,
    EXPLOSIVE_STYLE_ICE,
    EXPLOSIVE_STYLE_STUN,
    EXPLOSIVE_STYLE_LAUNCH,

    NUM_EXPLOSIVE_STYLES
};

CGUID GetMeshGUID(CThing* thing);
bool IsExplosiveMesh(CThing* thing);

s32 GetExplosiveType(CThing* thing);
s32 GetExplosiveStyle(CThing* thing);

void SetExplosiveStyle(CThing* thing, s32 type_index, s32 style_index);

extern bool InitializeExplosiveStyles();

#endif // EXPLOSIVE_STYLES_H