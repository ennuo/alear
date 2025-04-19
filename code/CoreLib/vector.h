#ifndef VECTOR_H
#define VECTOR_H


#include "mem_allocator.h"
#include <new>

template <typename T>
class CBaseVector {
public:
	inline CBaseVector() 
	{
		this->Data = NULL;
		this->Size = 0;
		this->MaxSize = 0;
	}

	inline bool empty() const { return this->Size == 0; }
	inline u32 size() const { return this->Size; }
	inline u32 max_size() const { return this->MaxSize; };
	inline u32 capacity() const { return this->MaxSize - this->Size; }

	inline T* begin() const { return Data; }
	inline T* end() const { return Data + Size; }
	inline T& operator[](int index) const { return this->Data[index]; }
	inline T& front() const { return this->Data[0]; }
	inline T& back() const { return this->Data[this->Size - 1]; }
	
	inline u32& GetSizeForSerialisation() { return Size; }
protected:
	T* Data;
	u32 Size;
	u32 MaxSize;
};

/* vector.h: 101 */
template <typename T, typename Allocator = CAllocatorMM>
class CBaseVectorPolicy : public CBaseVector<T>  {
public:
	inline CBaseVectorPolicy() : CBaseVector<T>() {}
};

/* vector.h: 152 */
template <typename Allocator = CAllocatorMM>
struct AllocBehaviour {
public:
	static bool Reserve(void** data, u32& max_size, u32 new_max_size, u32 sizeof_t) 
	{
		if (max_size < new_max_size) 
		{
			u32 count = Allocator::ResizePolicy(max_size, new_max_size, sizeof_t);
			void* mem = Allocator::Realloc(gVectorBucket, *data, sizeof_t * count);
			if (mem != NULL) 
			{
				*data = mem;
				max_size = count;
				return true;
			}

			return false;
		}
		return true;
	}

	static bool Resize(void** data, u32& max_size, u32& size, u32 new_size, u32 sizeof_t) {
		bool reserved = AllocBehaviour<Allocator>::Reserve(data, max_size, new_size, sizeof_t);
		if (reserved) size = new_size;
		return reserved;
	}
};

template <typename T, typename Allocator = CAllocatorMM>
class CParasiticVector : public CBaseVectorPolicy<T, Allocator> {
public:
	inline CParasiticVector() : CBaseVectorPolicy<T, Allocator>() {}
	inline CParasiticVector(T* data, u32 size, u32 max_size)
	{
		this->Data = data;
		this->Size = size;
		this->MaxSize = max_size;
	}
};

/* vector.h: 189 */
template <typename T, typename Allocator = CAllocatorMM>
class CRawVector : public CBaseVectorPolicy<T, Allocator> {
public:
	inline CRawVector() : CBaseVectorPolicy<T, Allocator>() {}
	inline CRawVector(u32 capacity) 
	{
		this->try_reserve(capacity);
	}

	inline CRawVector(CRawVector<T, Allocator> const& vec) : CBaseVectorPolicy<T, Allocator>()
	{
		*this = vec;
	}

	inline ~CRawVector() 
	{
		Allocator::Free(gVectorBucket, this->Data);
	}

	inline CRawVector<T, Allocator>& operator=(CRawVector<T, Allocator> const& vec)
	{
		this->clear();
		
		if (vec.Data != NULL)
		{
			this->Data = (T*)Allocator::Malloc(gVectorBucket, vec.Size * sizeof(T));
			memcpy(this->Data, vec.Data, vec.Size * sizeof(T));
			this->Size = vec.Size;
			this->MaxSize = vec.Size;
		}

		return *this;
	}

	inline void push_front(T const& element)
	{
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		memmove((void*)(this->Data + 1), (void*)this->Data, this->Size * sizeof(T));
		this->Data[0] = element;
		this->Size++;
	}
	
	inline void push_back(T const& element) 
	{
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		this->Data[this->Size++] = element;
	}

	inline T* erase(T* i) 
	{
		unsigned int return_index = i - this->Data;
		unsigned int copy_index = return_index + 1;
		
		if (copy_index < this->Size)
			memmove(this->Data + return_index, this->Data + copy_index, (this->Size - copy_index) * sizeof(T));
		
		this->Size--;
		return this->Data + return_index;
	}

	inline void clear()
	{	
		Allocator::Free(gVectorBucket, this->Data);
		this->Data = NULL;
		this->Size = 0;
		this->MaxSize = 0;
	}

	bool try_reserve(u32 new_max_size) 
	{
		return AllocBehaviour<Allocator>::Reserve((void**)&this->Data, this->MaxSize, new_max_size, sizeof(T));
	}

	bool try_resize(u32 new_size) 
	{
		return AllocBehaviour<Allocator>::Resize((void**)&this->Data, this->MaxSize, this->Size, new_size, sizeof(T));
	}

};

/* vector.h: 386 */
template <typename T, typename Allocator = CAllocatorMM>
class CVector : public CBaseVectorPolicy<T> {
public:
	inline CVector() : CBaseVectorPolicy<T, Allocator>() {}
	inline CVector(u32 capacity) {
		this->try_reserve(capacity);
	}

	inline CVector(CVector<T, Allocator> const& vec) : CBaseVectorPolicy<T, Allocator>()
	{
		*this = vec;
	}

	inline CVector<T, Allocator>& operator=(CVector<T, Allocator> const& vec)
	{
		if (this->Data != NULL) this->clear();

		if (vec.Data != NULL)
		{
			this->Data = (T*)Allocator::Malloc(gVectorBucket, vec.Size * sizeof(T));
			for (int i = 0; i < vec.Size; ++i)
			{
				T& element = this->Data[i];
				new (&element) T();
				this->Data[i] = vec.Data[i];
			}
			
			this->Size = vec.Size;
			this->MaxSize = vec.Size;
		}

		return *this;
	}

	inline ~CVector() 
	{
		if (this->Size != 0)
		{
			for (unsigned int i = 0; i < this->Size; ++i)
				(&this->Data[i])->~T();
		}

		Allocator::Free(gVectorBucket, this->Data);
	}

	inline void push_back(T const& element) 
	{
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		new (&this->Data[this->Size]) T();
		this->Data[this->Size++] = element;
	}

	inline T* erase(T* i) 
	{
		unsigned int return_index = i - this->Data;
		unsigned int index = return_index;

		while (index < this->Size - 1)
		{
			T& prev = this->Data[index];
			T& next = this->Data[++index];
			
			(&prev)->~T();
			new (&prev) T();
			prev = next;
		}

		
		this->Size--;
		return this->Data + return_index;
	}

	inline void clear()
	{
		if (this->Size != 0)
		{
			for (unsigned int i = 0; i < this->Size; ++i)
				(&this->Data[i])->~T();
		}

		Allocator::Free(gVectorBucket, this->Data);
		this->Data = NULL;
		this->Size = 0;
		this->MaxSize = 0;
	}

	bool try_reserve(u32 new_max_size) 
	{
		if (this->MaxSize < new_max_size) 
		{
			u32 count = Allocator::ResizePolicy(this->MaxSize, new_max_size, sizeof(T));
			T* data = (T*) Allocator::Malloc(gVectorBucket, count * sizeof(T));
			if (data != NULL) 
			{
				if (this->Size != 0) 
				{
					for (u32 i = 0; i < this->Size; ++i)
					{
						T& old = this->Data[i];
						new (data + i) T();
						data[i] = old;
						(&old)->~T();
					}

					Allocator::Free(gVectorBucket, this->Data);
				}
				this->MaxSize = new_max_size;
				this->Data = data;
				return true;
			}
			return false;
		}

		return true;
	}

	bool try_resize(u32 new_size) 
	{
		if (try_reserve(new_size)) 
		{
			for (int i = this->Size; i < new_size; ++i)
				new (this->Data + i) T();
			this->Size = new_size;
			return true;
		}

		return false;
	}
};

typedef CRawVector<char, CAllocatorMMAligned128> ByteArray;

#endif // VECTOR_H