#include "GuidHash.h"

CHash CHash::ZERO;
const char* HexChars = "0123456789abcdef";
void CHash::ConvertToHex(char(&hash)[HASH_HEX_STRING_LENGTH]) const
{
    for (int i = 0; i < HASH_BUF_LENGTH; ++i)
    {
        u8 b = Bytes[i];
        hash[i * 2] = HexChars[b >> 4];
        hash[(i * 2) + 1] = HexChars[b & 0xf];
    }
}