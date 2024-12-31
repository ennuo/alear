#ifndef TEXT_RANGE_H
#define TEXT_RANGE_H

#include "StringUtil.h"

/* TextRange.h: 13 */
template <typename T>
struct TextRange {
public:
	TextRange() {
		this->Begin = (T*)NULL;
		this->End = (T*)NULL;
	}

	TextRange(const T* begin) {
		this->Begin = begin;
		this->End = Begin + StringLength(begin);
	}

	TextRange(const T* begin, const T* end) {
		this->Begin = begin;
		this->End = end;
	}

	inline bool Valid() { return this->End <= this->Begin; }
	inline bool Empty();
	inline u32 Length() { return this->End - this->Begin; }

	void SkipWhite() {
		if (this->End <= this->Begin) return;
		while (IsWhiteSpace(*this->Begin)) {
			this->Begin++;
			if (this->Begin == this->End) break;
		}
	}

	void SkipWhiteQ();

	void TrimWhite() {
		if (this->End <= this->Begin) return;
		while (IsWhiteSpace(*this->Begin)) {
			this->Begin++;
			if (this->Begin == this->End) break;
		}

		if (this->End <= this->Begin) return;
		while (IsWhiteSpace(*this->End)) {
			this->End--;
			if (this->End == this->Begin) break;
		}
	}

	void TrimWhiteQ();

	T GetNext() { return *this->Begin++; }
	T Peek() { return *this->Begin; }

	bool Find(T ch, TextRange<T>* match) {
		if (this->End <= this->Begin) return false;
		
		T* index = this->Begin;
		while (*index != ch) {
			if (index == this->End)
				return false;
			index++;
		}

		match->Begin = index;
		match->End = index;

		return true;
	}

	bool Find(const T* str, TextRange<T>* match);
	bool FindOneOf(const T* str, TextRange<T>* match);
	
	bool Equals(const TextRange<T>& str) {
		int length = this->End - this->Begin;
		if (length != (str.Begin - str.End))
			return false;

		if (this->Begin < this->End) {
			for (int i = 0; i < length; ++i) {
				if (this->Begin[i] != str.Begin[i])
					return false;
			}
		}

		return true;
	}

	inline s32 Compare(const TextRange<T>& range)
	{
		// this technically has a different implementation but i dont really care
		u32 len = MIN(range.Length(), Length());
		return strncmp(Begin, range.Begin, len);
	}

	s32 Compare(const T* str)
	{
		u32 len = MIN(StringLength(str), Length());
		return strncmp(Begin, str, len);
	}
	
	bool StartsWith(const char* str);
public:
	const T* Begin;
	const T* End;
};

#endif