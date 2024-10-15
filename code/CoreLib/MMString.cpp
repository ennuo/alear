#include "MMString.h"

#include "hook.h"

// These functions do exactly the same thing
MH_DefineFunc(MultiByteToWChar, 0x00590fa0, TOC1, wchar_t const*, MMString<wchar_t>& dst, char const* src, char const* src_end);
MH_DefineFunc(MultiByteToTChar, 0x00590fa0, TOC1, tchar_t const*, MMString<tchar_t>& dst, char const* src, char const* src_end);
