#ifndef VECTOR_H
#define VECTOR_H


#include "mem_allocator.h"

template <typename T>
class CBaseVector {
public:
	inline CBaseVector() {
		this->Data = NULL;
		this->Size = 0;
		this->MaxSize = 0;
	}

	inline bool empty() { return this->Size == 0; }
	inline u32 size() { return this->Size; }
	inline u32 max_size() { return this->MaxSize; };
	inline u32 capacity() { return this->MaxSize - this->Size; }

	inline T* begin() { return &this->Data[0]; }
	inline T* end() { return &this->Data[this->Size - 1]; }
	inline T& operator[](int index) { return this->Data[index]; }
	inline T& front() { return this->Data[0]; }
	inline T& back() { return this->Data[this->Size - 1]; }
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
	static bool Reserve(void** data, u32* max_size, u32 new_max_size, u32 sizeof_t) {
		if (*max_size < new_max_size) {
			u32 count = Allocator::ResizePolicy(*max_size, new_max_size, sizeof_t);
			void* mem = Allocator::Realloc(gVectorBucket, *data, sizeof_t * count);
			if (mem != NULL) {
				*data = mem;
				*max_size = count;
				return true;
			}
			return false;
		}
		return true;
	}

	static bool Resize(void** data, u32* max_size, u32* size, u32 new_size, u32 sizeof_t) {
		bool reserved = AllocBehaviour<Allocator>::Reserve(data, max_size, new_size, sizeof_t);
		if (reserved)
			*size = new_size;
		return reserved;
	}
};

/* vector.h: 189 */
template <typename T, typename Allocator = CAllocatorMM>
class CRawVector : public CBaseVectorPolicy<T, Allocator> {
public:
	inline CRawVector() : CBaseVectorPolicy<T, Allocator>() {}
	inline CRawVector(u32 capacity) {
		this->try_reserve(capacity);
	}
	inline ~CRawVector() {
		if (this->Data != NULL) {
			Allocator::Free(gVectorBucket, this->Data);
			this->Data = NULL;
		}
		this->Size = 0;
		this->MaxSize = 0;
	}

	inline void push_back(T element) {
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		this->Data[this->Size++] = element;
	}

	bool try_reserve(u32 new_max_size) {
		return AllocBehaviour<Allocator>::Reserve((void**) &this->Data, &this->MaxSize, new_max_size, sizeof(T));
	}

	bool try_resize(u32 new_size) {
		return AllocBehaviour<Allocator>::Resize((void**) &this->Data, &this->MaxSize, &this->Size, new_size, sizeof(T));
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

	inline ~CVector() {
		if (this->Data != NULL) {
			Allocator::Free(gVectorBucket, this->Data);
			this->Data = NULL;
		}
		this->Size = 0;
		this->MaxSize = 0;
	}

	inline void push_back(T element) {
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		this->Data[this->Size++] = element;
	}

	inline T* erase(T* i) {
		unsigned int return_index = (i - this->Data) / sizeof(T);
		unsigned int copy_index = return_index;
		unsigned int index = return_index;
		while (index != (this->Size - 1))
			this->Data[copy_index++] = this->Data[++index];
		this->Size--;
		return this->Data + index;
	}

	bool try_reserve(u32 new_max_size) {
		if (this->MaxSize < new_max_size) {
			u32 count = Allocator::ResizePolicy(this->MaxSize, new_max_size, sizeof(T));
			T* data = (T*) Allocator::Malloc(gVectorBucket, count * sizeof(T));
			if (data != NULL) {
				if (this->Size != 0) {
					for (u32 i = 0; i < this->Size; ++i)
						memcpy(&data[i], &this->Data[i], sizeof(T));
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

	bool try_resize(u32 new_size) {
		if (try_reserve(new_size)) {
			for (unsigned int i = this->Size; i < new_size; ++i)
				new (&this->Data[i]) T();
			this->Size = new_size;
			return true;
		}
		return false;
	}
};

typedef CRawVector<char> ByteArray;

#endif // VECTOR_H