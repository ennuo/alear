#include "StringUtil.h"

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
size_t StringCopy(T* dst, const T* src, unsigned int size)
{
    unsigned int num = size;
	T* ptr = (T*) src;
	while (*src && num--)
		*dst++ = *src++;
	*dst = '\0';
	return (ptr - src) - 1;
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

template <typename T, unsigned int size>
inline size_t StringCopy(T dst[size], T const* src) 
{
	return StringCopy(dst, src, size);
}