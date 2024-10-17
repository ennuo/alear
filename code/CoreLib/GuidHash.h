#ifndef GUID_HASH_H
#define GUID_HASH_H

#include <string.h>



// ALL OF THIS NEEDS TO BE CLEANED UP !!!!

/* GuidHash.h: 22 */
class CGUID {
public:
	static CGUID ZERO;
	
	inline CGUID(const int& value) { guid = (u32)value; }
	inline CGUID(const u32& value) { guid = value; }
	operator u32() const { return guid; }

	u32 guid;

	inline CGUID() { guid = 0; }
	inline void Clear() { guid = 0; };
	inline operator bool() const { return guid != 0; }
	inline bool operator !() const { return guid == 0; }
	inline int Compare(const CGUID* rhs) { return rhs->guid - guid; }
	inline bool operator==(const CGUID* rhs) const { return guid == rhs->guid; }
	inline bool operator!=(const CGUID* rhs) const { return guid != rhs->guid; }
	inline bool operator<(const CGUID* rhs) const { return guid < rhs->guid; }
	inline bool operator<=(const CGUID* rhs) const { return guid <= rhs->guid; }
	inline bool operator>(const CGUID* rhs) const { return guid > rhs->guid; }
	inline bool operator>=(const CGUID* rhs) const { return guid >= rhs->guid; }

	inline bool operator==(int rhs) const { return guid == rhs; }
};

/* GuidHash.h: 72 */
class CHash {
private:
    u8 Bytes[20];
public:
	static CHash ZERO;
	static CHash EMPTY_STRING;

	inline CHash()
	{
		Clear();
	}
	
	// CHash(u8* hash) { memcpy(Bytes, hash, 0x14); }
	// CHash(u8* data, size_t size);
	// CHash(const char* hex, size_t size);
	// CHash(ByteArray* data);

	inline void Clear() { memset(Bytes, 0, 0x14); }
	// void* GetBuf() { return &Bytes; }

	inline int Compare(CHash* B) { return memcmp(this, B, 0x14); }

	inline bool operator==(CHash* rhs) { return this->Compare(rhs) == 0; }
	inline bool operator!=(CHash* rhs) { return this->Compare(rhs) != 0; }

	// void ConvertToHex(char* c);

	// static u8 ToHexChar(u8 i);
	// static u8 FromHexChar(char c);

	// bool TryConvertFromHex(char* hex);
	// bool TryAppendHex(char* hex, size_t size);
	// void ConvertFromNibble(char* nibble);
	// uint8_t Nybble(int b);

	bool IsSet() { return this != &ZERO; }
};


#endif