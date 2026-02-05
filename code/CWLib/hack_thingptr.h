#ifndef CTHING_PTR
#define CTHING_PTR

class CThing;
class CThingPtr {
public:
    CThingPtr();
    CThingPtr(CThing* thing);
    ~CThingPtr();
    void Unset();
    void Set(CThing* thing);
    CThingPtr& operator=(CThingPtr const& rhs);
    CThingPtr& operator=(CThing* rhs);
    inline operator CThing*() const { return Thing; }
    inline CThing* operator->() const { return Thing; }
    inline CThing* GetThing() const { return Thing; }
private:
    CThing* Thing;
    CThingPtr* Next;
    CThingPtr* Prev;
};

#endif // CTHING_PTR