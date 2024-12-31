#include "StringLookupTable.h"
#include <hook.h>

MH_DefineFunc(CStringLookupTable_AddString, 0x0022fd50, TOC0, u32, CStringLookupTable*, u32 key);
u32 CStringLookupTable::AddString(u32 key)
{
    return CStringLookupTable_AddString(this, key);
}