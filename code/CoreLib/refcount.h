#ifndef REFCOUNT_H
#define REFCOUNT_H

#include <cell/atomic.h>



typedef void (*RemoveRefFunc)(void*);

/* refcount.h: 13 */
class CBaseCounted {
protected:
	virtual ~CBaseCounted() {}
public:
	int AddRef() const { return cellAtomicIncr32((uint32_t*) &RefCount); }
	int Release() const { return cellAtomicDecr32((uint32_t*) &RefCount); }
	int GetRefCount() const { return RefCount; }

	volatile int RefCount;
};

template <class T>
class _NoAddRefRelease : public T {
public:
	int AddRef() const { return this->GetRefCount(); }
	int Release() const { return this->GetRefCount(); }
	int AddWeakRef() const { return this->GetRefCount(); }
	int ReleaseWeakRef() const { return this->GetRefCount(); }
};

/* refcount.h: 55 */
template <typename T>
class CP {
public:
	T& operator*() const { return *Ref; }
	_NoAddRefRelease<T>* operator->() const { return (_NoAddRefRelease<T>*)Ref; }
	operator T*() const { return Ref; }
	T* GetRef() const { return Ref; }
public:
	CP() : Ref(NULL) {}

	CP(T* ptr) { this->CopyFrom(ptr); }

	CP(const CP<T>& rhs) { this->CopyFrom(rhs.Ref); }

	~CP() 
	{
		if (Ref == NULL) return;
		if ((Ref->Release() - 1) == 0)
			delete Ref;
	}
public:
	CP<T>& operator=(T const* rhs)
	{
		if (Ref) 
		{
			if ((Ref->Release() - 1) == 0)
				delete Ref;
		}

		Ref = (T*)rhs;

		if (Ref)
			Ref->AddRef();

		return *this;
	}

	CP<T>& operator=(CP<T> const& rhs) 
	{
		if (Ref) 
		{
			if ((Ref->Release() - 1) == 0)
				delete Ref;
		}

		Ref = rhs.Ref;

		if (Ref)
			Ref->AddRef();

		return *this;
	}
public:
	bool operator!() const { return !Ref; }
	bool operator==(T* rhs) const { return Ref == rhs.Ref; }
	bool operator!=(T* rhs) const { return Ref != rhs.Ref; }
public:
	void CopyFrom(T* ptr)
	{
		Ref = ptr;
		if (ptr != NULL) ptr->AddRef();
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

	StaticCP(T* ptr) { this->CopyFrom(ptr); }
	StaticCP(const StaticCP<T>& rhs) { this->CopyFrom(rhs.Ref); }
	
	static void RemoveRef(void* ptr)
	{
		(*(StaticCP<T>*)ptr) = (T*)NULL;
	}
public:
	StaticCP<T>& operator=(T const* rhs)
	{
		if (this->Ref) 
		{
			if ((this->Ref->Release() - 1) == 0)
				delete this->Ref;
		}

		this->Ref = (T*)rhs;

		if (this->Ref)
			this->Ref->AddRef();

		return *this;
	}

	StaticCP<T>& operator=(StaticCP<T> const& rhs) 
	{
		if (this->Ref) 
		{
			if ((this->Ref->Release() - 1) == 0)
				delete this->Ref;
		}

		this->Ref = rhs.Ref;

		if (this->Ref)
			this->Ref->AddRef();

		return *this;
	}
public:
    RemoveRefFunc RemoveRefPtr;
    StaticCPForm* NextPtr;
};

#endif