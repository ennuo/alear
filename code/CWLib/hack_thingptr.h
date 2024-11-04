#ifndef CTHING_PTR
#define CTHING_PTR

class CThing;
class CThingPtr {
public:
    CThing* Thing;
    CThingPtr* Next;
    CThingPtr* Prev;
public:
    inline CThingPtr() : Thing(NULL), Next(NULL), Prev(NULL)
    {

    }
    
    void Unset();
    void Set(CThing* thing);

    inline ~CThingPtr()
    {
        Unset();
    }

    inline CThingPtr& operator=(CThingPtr const& rhs) 
    { 
        Set(rhs.Thing);
        return *this;
    }

    inline CThingPtr& operator=(CThing* rhs) 
    { 
        Set(rhs);
        return *this;
    }

    inline operator CThing*() const { return Thing; }
    inline CThing* operator->() const { return Thing; }
    inline CThing* GetThing() const { return Thing; }
};

#endif // CTHING_PTR