#include "ResourceTranslationTable.h"
#include "hook.h"

MH_DefineFunc(MakeLamsKeyID, 0x00023400, TOC0, u32, const char* prefix, const char* suffix);

MH_DefineFunc(RTranslationTable_GetText, 0x000959a0, TOC0, bool, RTranslationTable*, u32, tchar_t const*& out);
bool RTranslationTable::GetText(u32 key, tchar_t const*& out)
{
    return RTranslationTable_GetText(this, key, out);
}