#ifndef RESOURCE_POINTER_H
#define RESOURCE_POINTER_H

// check how this is actually meant to work later

template <typename T>
class WR {
public:
	bool operator!() const { return !Ref; }
	bool operator==(const T* rhs) const { return Ref == rhs.Ref; }
	bool operator!=(const T* rhs) const { return Ref != rhs.Ref; }
    
	const T& operator*() const { return *Ref; }
	T* operator->() { return Ref; }
	operator T*() { return Ref; }
public:
	WR() { Ref = NULL; }
protected:
    T* Ref;
};


#endif // RESOURCE_POINTER_H