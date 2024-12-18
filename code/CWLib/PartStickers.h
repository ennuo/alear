#ifndef PART_STICKERS_H
#define PART_STICKERS_H

#include <vector.h>
#include "Part.h"
#include "SceneGraph.h"

class PStickers : public CPart {
public:
    CVector<CDecal> Decals;
    CVector<CDecal> CostumeDecals[14];
};


#endif // PART_STICKERS_H