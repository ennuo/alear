#include "StringUtil.h"
#include <printf.h>
#include <hook.h>

size_t StringLength(const char* s)
{
    return strlen(s);
}

size_t StringLength(const wchar_t* s)
{
	return wcslen(s);
}

size_t StringLength(const tchar_t* s)
{
	return wcslen((const wchar_t*)s);
}

int StringCompare(const char* a, const char* b)
{
	return strcmp(a, b);
}

size_t StringCompareN(const char* a, const char* b, size_t len)
{
	return StringCompareN(a, b, len);
}

int StringICompare(const char* a, const char* b)
{
	return strcasecmp(a, b);
}

int StringICompareN(const char* a, const char* b, size_t len)
{
	return strncasecmp(a, b, len);
}

template<typename T>
size_t StringCopy(T* dst, T const* src, unsigned int size)
{
    unsigned int num = size;
	T* ptr = (T*) src;
	while (*src && num--)
		*dst++ = *src++;
	*dst = '\0';
	return src - ptr;
}

unsigned int StringCopy(char* dst, const char* src, unsigned int size)
{
    return StringCopy<char>(dst, src, size);
}

unsigned int StringCopy(wchar_t* dst, const wchar_t* src, unsigned int size)
{
    return StringCopy<wchar_t>(dst, src, size);
}

unsigned int StringCopy(char* dst, char const* src, unsigned int size);
unsigned int StringCopy(wchar_t* dst, wchar_t const* src, unsigned int size);

size_t FormatStringVarArg(char* dst, unsigned int size, char const* format, va_list args)
{
	dst[size - 1] = '\0';
	size_t len = vsnprintf(dst, size, format, args);
	dst[size - 1] = '\0';
	return len;
}


MH_DefineFunc(FormatStringVarArg_wstr, 0x00590ac0, TOC1, size_t, wchar_t*, unsigned int, wchar_t const*, va_list);
size_t FormatStringVarArg(wchar_t* dst, unsigned int size, wchar_t const* format, va_list args)
{
	return FormatStringVarArg_wstr(dst, size, format, args);
}