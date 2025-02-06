#ifndef CHECKPOINT_STYLES_H
#define CHECKPOINT_STYLES_H

#include <GuidHash.h>

class CThing;
class PCheckpoint;

enum ECheckpointType {
    CHECKPOINT_ENTRANCE,
    CHECKPOINT_SINGLE,
    CHECKPOINT_DOUBLE,
    CHECKPOINT_INFINITE,

    NUM_CHECKPOINT_TYPES
};

enum ECheckpointStyle {
    CHECKPOINT_STYLE_CARDBOARD,
    CHECKPOINT_STYLE_WOOD,
    CHECKPOINT_STYLE_PLASTIC,
    CHECKPOINT_STYLE_CHROME,

    NUM_CHECKPOINT_STYLES
};

CGUID GetMeshGUID(CThing* thing);
bool IsCheckpointMesh(CThing* thing);

s32 GetCheckpointType(CThing* thing);
s32 GetCheckpointStyle(CThing* thing);
PCheckpoint* FindPartCheckpoint(CThing* thing);

void SetCheckpointStyle(CThing* thing, s32 type_index, s32 style_index);


#endif // CHECKPOINT_STYLES_H