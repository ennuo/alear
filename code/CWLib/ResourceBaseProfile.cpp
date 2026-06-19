#include "ResourceBaseProfile.h"

u32 CBaseProfile::AddString(u32 key)
{
    return StringTable.AddString(key);
}

bool CBaseProfile::GetStringFromIndex(u32 index, MMString<wchar_t>& out)
{
    return StringTable.GetString(index, out);
}