#ifndef STRING_UTIL_H
#define STRING_UTIL_H

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

#endif // STRING_UTIL_H