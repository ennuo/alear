#pragma once

#include <TextRange.h>
#include <MMString.h>

u32 MakeLamsKeyID(TextRangeA lams_key);
u32 MakeLamsKeyID(const char* prefix, const char* suffix);
const tchar_t* Translate(u32 key);
bool Translate(MMString<wchar_t>& dst, const char* lams_key);
bool TryTranslate(u32 key, tchar_t const*& out);
