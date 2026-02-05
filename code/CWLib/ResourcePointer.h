#ifndef RESOURCE_POINTER_H
#define RESOURCE_POINTER_H

#include <refcount.h>

template <typename T>
class WR {
public:
	T& operator*() const { return *Ref; }
	_NoAddRefRelease<T>* operator->() const { return (_NoAddRefRelease<T>*)Ref; }
	operator T*() const { return Ref; }
	T* GetRef() const { return Ref; }
public:
	WR() : Ref(NULL) {}

	WR(T* ptr) { this->CopyFrom(ptr); }

	WR(const WR<T>& rhs) { this->CopyFrom(rhs.Ref); }

	~WR() 
	{
		if (Ref == NULL) return;
		if ((Ref->ReleaseWeakRef() - 1) == 0)
			delete Ref;
	}
public:
	WR<T>& operator=(T const* rhs)
	{
		if (Ref) 
		{
			if ((Ref->ReleaseWeakRef() - 1) == 0)
				delete Ref;
		}

		Ref = (T*)rhs;

		if (Ref)
			Ref->AddWeakRef();

		return *this;
	}

	WR<T>& operator=(WR<T> const& rhs) 
	{
		if (Ref) 
		{
			if ((Ref->ReleaseWeakRef() - 1) == 0)
				delete Ref;
		}

		Ref = rhs.Ref;

		if (Ref)
			Ref->AddWeakRef();

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
		if (ptr != NULL) ptr->AddWeakRef();
	}
protected:
    T* Ref;
};


#endif // RESOURCE_POINTER_H