#include "thing.h"
#include <hook.h>

MH_DefineFunc(CThing_ctor, 0x00020128, TOC0, void, CThing*);
CThing::CThing()
{
    CThing_ctor(this);
}

MH_DefineFunc(CThing_dtor, 0x0002250c, TOC0, void, CThing*);
CThing::~CThing()
{
    CThing_dtor(this);
}

MH_DefineFunc(CThing_AddPart, 0x00020770, TOC0, void, CThing*, EPartType);
void CThing::AddPart(EPartType type)
{
    CThing_AddPart(this, type);
}

MH_DefineFunc(CThing_SetWorld, 0x0001f5ec, TOC0, void, CThing*, PWorld*, u32);
void CThing::SetWorld(PWorld* world, u32 preferred_uid)
{
    CThing_SetWorld(this, world, preferred_uid);
}

// not technically supposed to be here but i dont care
void CThingPtr::Unset()
{
    if (Thing != NULL)
    {
        if (Next != NULL) Next->Prev = Prev;
        if (Prev == NULL) Next->Prev = Next;
        else Prev->Next = Next;
    }

    Thing = NULL;
    Next = NULL;
    Prev = NULL;
}

void CThingPtr::Set(CThing* thing)
{
    Unset();

    Thing = thing;
    if (Thing != NULL)
    {
        Next = Thing->FirstPtr;
        Thing->FirstPtr = this;
        if (Next != NULL) Next->Prev = this;
        Prev = NULL;
    }
    else
    {
        Next = NULL;
        Prev = NULL;
    }
}