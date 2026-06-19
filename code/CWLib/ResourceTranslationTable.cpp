#include <ResourceTranslationTable.h>
#include <algorithm>

bool RTranslationTable::GetText(u32 key, tchar_t const*& out)
{
    Index* index = std::lower_bound(IndexStart, IndexEnd, key, SSortByIndex());
    if (index != IndexEnd && index->Hash == key)
    {
        out = index->Text;
        return true;
    }

    return false;
}