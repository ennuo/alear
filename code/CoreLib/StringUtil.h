#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <printf.h>
#include <stdarg.h>

size_t StringLength(const char* str);
size_t StringLength(const wchar_t* s);
size_t StringLength(const tchar_t* s);
int StringCompare(const char* a, const char* b);
size_t StringCompareN(const char* a, const char* b, size_t len);
int StringICompare(const char* a, const char* b);
int StringICompareN(const char* a, const char* b, size_t len);


unsigned int StringCopy(char* dst, char const* src, unsigned int size);
unsigned int StringCopy(wchar_t* dst, wchar_t const* src, unsigned int size);

/* StringUtil.h: 56 */
template <typename T, unsigned int size>
inline size_t StringCopy(T dst[size], T const* src) 
{
	return StringCopy((T*)dst, src, size);
}

inline bool IsWhiteSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline bool IsWhiteSpace(wchar_t c)
{
	return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

inline bool StringIsNullOrEmpty(char* str) {
	return str == NULL || *str == '\0';
}

size_t FormatStringVarArg(char* dst, unsigned int size, char const* format, va_list args);
size_t FormatStringVarArg(wchar_t* dst, unsigned int size, wchar_t const* format, va_list args);


template <unsigned int size>
size_t FormatString(wchar_t dst[size], wchar_t* format, ...)
{
	va_list args;
    va_start(args, format);
	size_t len = FormatStringVarArg(dst, size, format, args);
	va_end(args);
	return len;
}


/* StringUtil.h: 84 */
template <u32 size>
inline size_t FormatString(char (&dst)[size], const char* format, ...)
{
	va_list args;
	va_start(args, format);
	size_t len = vsnprintf(dst, size, format, args);
	va_end(args);
	return len;
}


#endif // STRING_UTIL_H