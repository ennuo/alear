#ifndef MMSTRING_H
#define MMSTRING_H


#include "StringUtil.h"

template <typename T>
class StringTraits {
public:
    static T* Malloc(size_t c)
    {
        return new T[c];
    }
};

template <typename T>
class MMString {
    static const u32 LOCAL_STORE_BYTES = 16;
    static const u32 LOCAL_STORE_CHARS = (LOCAL_STORE_BYTES - sizeof(T)) / sizeof(T);
public:
    MMString()
    {
        LocalData.LocalStoreFlag = LOCAL_STORE_CHARS;
        LocalBuffer[0] = 0;
    }

    MMString(const T* b, const T* e)
    {
        Construct(b, (size_t)(e - b));
    }

    MMString(const T* s, size_t l)
    {
        Construct(s, l);
    }

    MMString(const T* s)
    {
        size_t l = StringLength(s);
        Construct(s, l);
    }

    MMString(MMString<char>* s)
    {
        Construct(s->c_str(), s->size());
    }

    ~MMString()
    {
        if (!IsUsingLocalData())
            delete HeapData.Buffer;
    }

    inline bool CanUseLocalData(size_t l)
    {
        return l <= LOCAL_STORE_CHARS;
    }

    inline bool IsUsingLocalData()
    {
        return LocalData.LocalStoreFlag != (T)-1;
    }

    inline T MakeLocalStoreFlag(size_t l)
    {
        return (l > LOCAL_STORE_CHARS) ? (T)-1 : LOCAL_STORE_CHARS - l;
    }

    void Construct(const T* s, size_t l)
    {
        T flag = MakeLocalStoreFlag(l);
        T* data = LocalBuffer;
        if (flag == (T)-1)
        {
            data = StringTraits<T>::Malloc(l + 1);

            HeapData.Buffer = data;
            HeapData.Length = l;
            HeapData.Capacity = l; 
        }

        LocalData.LocalStoreFlag = flag;
        memmove(data, s, l * sizeof(T));
        data[l] = 0;
    }

    inline void clear()
    {
        if (!IsUsingLocalData())
            delete HeapData.Buffer;
        
        LocalData.LocalStoreFlag = LOCAL_STORE_CHARS;
        LocalBuffer[0] = 0;
    }

    inline bool empty()
    {
        return size() == 0;
    }

    MMString<T>* assign(MMString<T>* s)
    {
        if (this != s)
        {
            if (!IsUsingLocalData())
                delete HeapData.Buffer;
            Construct(s->c_str(), s->size());
        }

        return this;
    }

    MMString<T>* assign(const T* s, size_t l)
    {
        // If the string already has data in it...
        if (!IsUsingLocalData())
        {
            // If there's still enough space in the heap data,
            // just copy the string in
            if (l <= HeapData.Capacity)
            {
                memmove(HeapData.Buffer, s, l * sizeof(T));
                HeapData.Buffer[l] = 0;
                HeapData.Length = l;
                return this;
            }

            // Otherwise allocate a new buffer

            T* data = StringTraits<T>::Malloc(l + 1);
            memmove(data, s, l * sizeof(T));
            data[l] = 0;

            if (HeapData.Buffer != NULL)
                delete HeapData.Buffer;

            HeapData.Buffer = data;
            HeapData.Capacity = l;
            HeapData.Length = l;
        }
        // String is empty or uses local data, need to construct
        else Construct(s, l);

        return this;
    }

    inline int compare(T* s)
    {
        return StringCompare(c_str(), s);
    }

    inline size_t size()
    {
        if (IsUsingLocalData())
            return LOCAL_STORE_CHARS - LocalData.LocalStoreFlag;
        return HeapData.Length;
    }

    inline T* c_str()
    {
        return IsUsingLocalData() ? (T*)&LocalBuffer : HeapData.Buffer;
    }

private:
    union {
        T LocalBuffer[LOCAL_STORE_BYTES / sizeof(T)];
        struct {
            T _LocalBufferPad[LOCAL_STORE_CHARS];
            T LocalStoreFlag;
        } LocalData;
        struct {
            T* Buffer;
            size_t Length;
            size_t Capacity;
            u32 Dummy;
        } HeapData;
        u64 Bits[LOCAL_STORE_BYTES / sizeof(u64)];
    };
};

extern tchar_t const* (*MultiByteToTChar)(MMString<tchar_t>& dst, char const* src, char const* src_end);
extern wchar_t const* (*MultiByteToWChar)(MMString<wchar_t>& dst, char const* src, char const* src_end);

#endif // MMSTRING_H