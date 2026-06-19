#include "StringLookupTable.h"


MH_DefineFunc(CStringLookupTable_AddString, 0x0022fd50, TOC0, u32, CStringLookupTable*, u32 key);
u32 CStringLookupTable::AddString(u32 key)
{
    return CStringLookupTable_AddString(this, key);
}

u32 CStringLookupTable::GetSortIndex(u32 index) const
{
    if (index >= RawIndexToSortedIndex.size())
        return -1;
    return RawIndexToSortedIndex[index];
}

bool CStringLookupTable::GetString(u32 index, MMString<wchar_t>& str_out)
{
    if (index >= RawIndexToSortedIndex.size()) return false;
    index = RawIndexToSortedIndex[index];
    if (index >= StringList.size()) return false;
    str_out = StringList[index].WString;
    return true;
}