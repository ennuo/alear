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
	inline int Compare(CGUID const& rhs) { return rhs.guid - guid; }
	inline bool operator==(CGUID const& rhs) const { return guid == rhs.guid; }
	inline bool operator!=(CGUID const& rhs) const { return guid != rhs.guid; }
	inline bool operator<(CGUID const& rhs) const { return guid < rhs.guid; }
	inline bool operator<=(CGUID const& rhs) const { return guid <= rhs.guid; }
	inline bool operator>(CGUID const& rhs) const { return guid > rhs.guid; }
	inline bool operator>=(CGUID const& rhs) const { return guid >= rhs.guid; }

	inline bool operator==(int rhs) const { return guid == rhs; }
};


const u32 HASH_HEX_STRING_LENGTH = 41;
const u32 HASH_BUF_LENGTH = 20;


/* GuidHash.h: 72 */
class CHash {
private:
    u8 Bytes[HASH_BUF_LENGTH];
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
	inline void* GetBuf() { return (void*)&Bytes; }

	inline int Compare(CHash const& b) const { return memcmp(Bytes, b.Bytes, HASH_BUF_LENGTH); }
	inline bool operator<(CHash const& rhs) const { return Compare(rhs) < 0; }
	inline bool operator>(CHash const& rhs) const { return Compare(rhs) > 0; }
	inline bool operator !() const { return memcmp(Bytes, ZERO.Bytes, HASH_BUF_LENGTH) == 0; }
	inline bool operator==(CHash const& rhs) const { return Compare(rhs) == 0; }
	inline bool operator!=(CHash const& rhs) const { return Compare(rhs) != 0; }

	void ConvertToHex(char(&)[HASH_HEX_STRING_LENGTH]) const;
	
	inline bool IsSet() const { return memcmp(Bytes, ZERO.Bytes, HASH_BUF_LENGTH) != 0; }
};


#endif