#ifndef PHOTO_METADATA_H
#define PHOTO_METADATA_H

#include <ReflectionVisitable.h>
#include <MMString.h>
#include <GuidHash.h>
#include <CalendarTime.h>
#include <vector.h>

#include "ResourceDescriptor.h"
#include "ResourceGFXTexture.h"
#include "ResourcePlan.h"

class CPlayerInPhoto { // file: 11
public:
    NetworkOnlineID OnlineID;
    MMString<tchar_t> DisplayName;
    v4 Bounds;
};

class CPhotoMetaData : public CReflectionVisitable { // file: 47
public:
    CResourceDescriptor<RTexture> Photo;
    CSlotID Level;
    MMString<tchar_t> LevelName;
    CHash LevelHash;
    CalendarTime TimeTaken;
    CVector<CPlayerInPhoto> Players;
};

struct SServerPhotoData {
    CResourceDescriptor<RTexture> Icon;
    CResourceDescriptor<RTexture> Sticker;
    CResourceDescriptor<RTexture> PhotoJPG;
    CResourceDescriptor<RPlan> Plan;
    CPhotoMetaData PhotoMetaData;
    v4 ScreenPos;
    u32 PhotoID;
};

#endif // PHOTO_METADATA_H