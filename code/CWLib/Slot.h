#ifndef SLOT_H
#define SLOT_H

#include <MMString.h>
#include <GuidHash.h>

#include "SlotID.h"
#include "ResourceDescriptor.h"
#include "network/NetworkUtilsNP.h"

class CSlot {
public:
    inline bool operator<(const CSlot& rhs) const
    {
        return SlotID < rhs.SlotID;
    }
public:
    CSlotID SlotID;
    CResourceDescriptor<RLevel> Level;
    CResourceDescriptor<RTexture> Icon;
    v4 Location;
    NetworkOnlineID AuthorID;
    MMString<wchar_t> AuthorName;
    MMString<char> TranslationTag;
    CSlotID PrimaryLinkLevel;
    CSlotID  Group;
    bool InitiallyLocked;
    u8 Shareable;
    CGUID BackgroundGUID;
    u32 DeveloperLevelType;
    u32 GameProgressionState;
    MMString<wchar_t> Name;
    MMString<wchar_t> Description;
};


#endif // SLOT_H