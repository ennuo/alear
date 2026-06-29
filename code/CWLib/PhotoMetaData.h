#pragma once

#include <ReflectionVisitable.h>
#include <ResourceDescriptor.h>
#include <MMString.h>
#include <GuidHash.h>
#include <SlotID.h>
#include <CalendarTime.h>
#include <vector.h>
#include <network/NetworkUtilsNP.h>

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

class SInventoryItemPhotoData : public CReflectionVisitable {
public:
    CResourceDescriptor<RTexture> Icon;
    CResourceDescriptor<RTexture> Sticker;
    CPhotoMetaData PhotoMetadata;
};
