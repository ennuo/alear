#ifndef REFCOUNT_H
#define REFCOUNT_H

#include <cell/atomic.h>



typedef void (*RemoveRefFunc)(void*);

/* refcount.h: 13 */
class CBaseCounted {
public:
	int AddRef() const { return cellAtomicIncr32((uint32_t*) &RefCount); }
	int Release() const { return cellAtomicDecr32((uint32_t*) &RefCount); }
	int GetRefCount() const { return RefCount; }

	volatile int RefCount;
};

/* refcount.h: 55 */
template <typename T>
class CP {
public:
	bool operator!() const { return !Ref; }
	bool operator==(const T* rhs) const { return Ref == rhs.Ref; }
	bool operator!=(const T* rhs) const { return Ref != rhs.Ref; }

	const T& operator*() const { return *Ref; }
	T* operator->() { return Ref; }
	operator T*() { return Ref; }
	T* GetRef() const { return Ref; }
public:
	CP() { Ref = NULL; }

	CP(T* ptr) { this->CopyFrom(ptr); }

	CP(const CP<T>& rhs)
	{
		Ref = rhs.Ref;
		if (Ref) Ref->AddRef();
	}

	~CP() 
	{
		if (Ref == NULL) return;
		if ((Ref->Release() - 1) == 0)
			delete Ref;
	}
public:
	CP<T>& operator=(CP<T>& rhs) 
	{
		if (Ref) {
			if ((Ref->Release() - 1) == 0)
				delete Ref;
		}
		Ref = rhs.Ref;

		if (Ref)
			Ref->AddRef();
	}

	CP<T>& operator=(T* rhs)
	{
		if (Ref) {
			if ((Ref->Release() - 1) == 0)
				delete Ref;
		}

		Ref = rhs;

		if (Ref)
			Ref->AddRef();
	}

public:
	void CopyFrom(T* ptr)
	{
		if (ptr) {
			ptr->AddRef();
			Ref = ptr;
		}
	}
protected:
    T* Ref;
};

/* refcount.h: 111 */
class StaticCPForm {
public:
    void* Ref;
    RemoveRefFunc RemoveRefPtr;
    StaticCPForm* NextPtr;
};

extern StaticCPForm* gStaticCPHead;

/* refcount.h: 121 */
template <typename T>
class StaticCP : public CP<T> {
public:
	StaticCP()
	{
		this->Ref = NULL;
		RemoveRefPtr = &RemoveRef;
		NextPtr = gStaticCPHead;
		gStaticCPHead = (StaticCPForm*)this;
	}
	
	static void RemoveRef(void* ptr)
	{
		(*(CP<T>*)ptr) = (T*)NULL;
	}
public:
    RemoveRefFunc RemoveRefPtr;
    StaticCPForm* NextPtr;
};

#endif