#ifndef CTHING_PTR
#define CTHING_PTR

class CThing;
class PWorld;

class CThingPtr {
public:
    CThingPtr();
    CThingPtr(const CThingPtr& rhs);
    CThingPtr(CThing* thing);
    CThingPtr(const CThingPtr* rhs);
    ~CThingPtr();
    void Unset();
    void Set(CThing* thing);
    CThingPtr& operator=(CThingPtr const& rhs);
    CThingPtr& operator=(CThing* rhs);
    inline operator CThing*() const { return Thing; }
    inline CThing* operator->() const { return Thing; }
    inline CThing* GetThing() const { return Thing; }
public:
    bool IsDeferred() const;
    void SetDeferred(u32 thing_uid);
    void Link(PWorld* world);
private:
    union
    {
        struct
        {
            CThing* Thing;
            CThingPtr* Next;
            CThingPtr* Prev;
        };
        struct
        {
            u32 ThingUID;
            u32 Magic;
            u32 Pad;
        };
    };
};

#endif // CTHING_PTR