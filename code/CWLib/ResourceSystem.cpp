#include "ResourceSystem.h"
#include "hook.h"

MH_DefineFunc(BlockUntilResourcesLoaded, 0x0008f174, TOC0, bool, CResource** resources, u32 count);
MH_DefineFunc(LoadResource, 0x0008e194, TOC0, CP<CResource>, CResourceDescriptorBase const&, CStreamPriority, u32, bool);