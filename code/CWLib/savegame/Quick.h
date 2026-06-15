#pragma once

#include <ResourceDescriptor.h>

struct SSaveKey {
    u32 Deprecated1[10];
    bool Copied;
    EResourceType RootType;
    u32 Deprecated2[3];
    CHash RootHash;
    u32 Deprecated3[10];
};
