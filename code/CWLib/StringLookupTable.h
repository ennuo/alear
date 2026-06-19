#ifndef STRING_LOOKUP_TABLE_H
#define STRING_LOOKUP_TABLE_H

#include <vector.h>
#include <MMString.h>

class CSortString {
public:
    u32 String;
    MMString<wchar_t> WString;
    u32 Index;
};

class CStringLookupTable {
public:
    u32 AddString(u32 key);
    u32 GetSortIndex(u32 index) const;
    bool GetString(u32 index, MMString<wchar_t>& out);
public:
    bool Unsorted;
    bool SortEnabled;
    bool SortSupressed;
    CRawVector<unsigned int> RawIndexToSortedIndex;
    CVector<CSortString> StringList;
};

#endif // STRING_LOOKUP_TABLE_H