#ifndef TEXT_RANGE_H
#define TEXT_RANGE_H

#include "StringUtil.h"

/* TextRange.h: 13 */
template <typename T>
struct TextRange {
public:
    TextRange() : Begin(), End()
    {

    }

    TextRange(const T* begin) : Begin(begin), End(begin + StringLength(begin))
    {

    }

    TextRange(const T* begin, const T* end) : Begin(begin), End(end)
    {

    }
public:
    bool Valid() const
    {
        return Begin < End;
    }

    bool Empty() const
    {
        return Begin == End;
    }

    u32 Length() const
    {
        return End - Begin;
    }

	void SkipWhite() 
	{
		if (this->End <= this->Begin) return;
		while (IsWhiteSpace(*this->Begin)) {
			this->Begin++;
			if (this->Begin == this->End) break;
		}
	}

	void SkipWhiteQ();

    void TrimWhite()
    {
        if (End <= Begin) return;
        while (Begin != End && IsWhiteSpace(*Begin)) Begin++;

        if (End <= Begin) return;
        while (End != Begin && IsWhiteSpace(*(End - 1))) End--;
    }

	void TrimWhiteQ();

    T GetNext()
    {
        return *Begin++;
    }
    
    T Peek() const
    {
        return *Begin;
    }

	bool Find(T ch, TextRange* match) const
	{
		if (this->End <= this->Begin) return false;
		
		T* index = this->Begin;
		while (*index != ch) 
		{
			if (index == this->End)
				return false;
			index++;
		}

		match->Begin = index;
		match->End = index;

		return true;
	}

	bool Find(const T* str, TextRange* match);
	bool FindOneOf(const T* str, TextRange* match);
	
	bool Equals(const TextRange& str) const
	{
		int length = this->Length();
		if (length != str.Length()) return false;

		if (this->Empty()) return true;

		for (u32 i = 0; i < length; ++i)
		{
			if (this->Begin[i] != str.Begin[i])
				return false;
		}

		return true;
	}

	inline s32 Compare(const TextRange& range) const
	{
		return StringCompareN(Begin, range.Begin, MIN(range.Length(), Length()));
	}

	s32 Compare(const T* str) const
	{
		return StringCompareN(Begin, str, MIN(StringLength(str), Length()));
	}
	
	bool StartsWith(const char* str);
public:
	const T* Begin;
	const T* End;
};

typedef TextRange<char> TextRangeA;

#endif