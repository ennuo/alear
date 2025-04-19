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

    inline CThingPtr(CThing* thing) : Thing(NULL), Next(NULL), Prev(NULL)
    {
        Set(thing);
    }

    inline CThingPtr(CThingPtr const& rhs) : Thing(NULL), Next(NULL), Prev(NULL)
    {
        *this = rhs;
    }
    
    void Unset();
    void Set(CThing* thing);

    inline ~CThingPtr()
    {
        Unset();
    }

    inline CThingPtr& operator=(CThingPtr const& rhs) 
    { 
        Unset();
        Set(rhs.Thing);
        return *this;
    }

    inline CThingPtr& operator=(CThing* rhs) 
    { 
        Unset();
        Set(rhs);
        return *this;
    }

    bool operator<(CThingPtr const& r) const;

    inline operator CThing*() const { return Thing; }
    inline CThing* operator->() const { return Thing; }
    inline CThing* GetThing() const { return Thing; }
};

#endif // CTHING_PTR