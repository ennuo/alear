#pragma once

#include <PoppetEnums.inl>

class CPoppetModeEx {
public:
    u64 SomeBullshit; // 0x8
    u32 RemoteModeCount; // 0x10
    u32 CollectionIdx; // 0x14
    u32 LastSelectedItemID; // 0x18
public:
    void Reset();
};

class CPoppetMode {
public:
    EPoppetMode Mode; // 0x0
    EPoppetSubMode SubMode; // 0x4
    CPoppetModeEx Ex; // 0x8
};
