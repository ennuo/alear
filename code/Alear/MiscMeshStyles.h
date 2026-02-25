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

enum EMagicEyeStyle {
    MAGIC_EYE_STYLE_NORMAL,
    MAGIC_EYE_STYLE_CUTE,
    MAGIC_EYE_STYLE_EVIL,

    NUM_MAGIC_EYE_STYLES
};

enum ELeverSwitchStyle {
    LEVER_SWITCH_STYLE_CARDBOARD,
    LEVER_SWITCH_STYLE_WOOD,
    LEVER_SWITCH_STYLE_PLASTIC,
    LEVER_SWITCH_STYLE_CHROME,

    NUM_LEVER_SWITCH_STYLES
};

enum ELeverSwitchType {
    LEVER_SWITCH_BINARY,
    LEVER_SWITCH_TRINARY,

    NUM_LEVER_SWITCH_TYPES
};

enum EBouncePadStyle {
    BOUNCE_PAD_STYLE_CARDBOARD,
    BOUNCE_PAD_STYLE_WOOD,
    BOUNCE_PAD_STYLE_PLASTIC,
    BOUNCE_PAD_STYLE_CHROME,

    NUM_BOUNCE_PAD_STYLES
};

enum ESpikePlateStyle {
    SPIKE_PLATE_STYLE_CARDBOARD,
    SPIKE_PLATE_STYLE_WOOD,
    SPIKE_PLATE_STYLE_PLASTIC,
    SPIKE_PLATE_STYLE_CHROME,

    NUM_SPIKE_PLATE_STYLES
};

enum ESpikePlateType {
    SPIKE_PLATE_SMALL,
    SPIKE_PLATE_LARGE,

    NUM_SPIKE_PLATE_TYPES
};

CGUID GetMeshGUID(CThing* thing);

bool IsMagicEyeMesh(CThing* thing);

s32 GetLevelKeyStyle(CThing* thing);
s32 GetMagicEyeStyle(CThing* thing);
s32 GetLeverSwitchStyle(CThing* thing);
s32 GetLeverSwitchType(CThing* thing);
s32 GetBouncePadStyle(CThing* thing);
s32 GetSpikePlateStyle(CThing* thing);
s32 GetSpikePlateType(CThing* thing);

void SetLevelKeyStyle(CThing* thing, s32 style_index);
void SetMagicEyeStyle(CThing* thing, s32 style_index);
void SetLeverSwitchStyle(CThing* thing, s32 type_index, s32 style_index);
void SetBouncePadStyle(CThing* thing, s32 style_index);
void SetSpikePlateStyle(CThing* thing, s32 type_index, s32 style_index);


#endif // MISC_MESH_STYLES_H