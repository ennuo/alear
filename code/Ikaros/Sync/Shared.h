#pragma once

#include <DebugLog.h>

#define SYNC_LOG(...) MMLogCh(DC_NETWORK, "sync: " __VA_ARGS__)

const u32 kSyncServerPort = 16723;
const u32 kMaxConcurrentDownloads = 4;