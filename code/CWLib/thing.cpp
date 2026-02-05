#include "thing.h"
#include <hook.h>

CThingPtr::~CThingPtr()
{
    Unset();
}

void CThingPtr::Unset()
{
    if (Thing != NULL)
    {
        if (Next != NULL) Next->Prev = Prev;
        if (Prev == NULL) Thing->FirstPtr = Next;
        else Prev->Next = Next;
    }

    Thing = NULL;
    Next = NULL;
    Prev = NULL;
}

void CThingPtr::Set(CThing* thing)
{
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

CThingPtr::CThingPtr() : Thing(NULL), Next(NULL), Prev(NULL)
{

}

CThingPtr::CThingPtr(CThing* thing) : Thing(NULL), Next(NULL), Prev(NULL)
{
    Set(thing);
}

CThingPtr& CThingPtr::operator=(CThingPtr const& rhs) 
{ 
    Unset();
    Set(rhs.Thing);
    return *this;
}

CThingPtr& CThingPtr::operator=(CThing* rhs) 
{ 
    Unset();
    Set(rhs);
    return *this;
}

MH_DefineFunc(GetWorldAngle, 0x00464f70, TOC1, float, CThing*);

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
    if (type == PART_TYPE_MATERIAL_OVERRIDE)
    {
        PMaterialOverride*& part = CustomThingData->PartMaterialOverride; 
        if (part != NULL) return; 

        part = new PMaterialOverride(); 
        part->SetThing_BECAUSE_I_HATE_CODING_CONVENTIONS_AND_NEED_TO_BE_SPANKED(this); 

        return; 
    }


    CThing_AddPart(this, type);
}

MH_DefineFunc(CThing_RemovePart, 0x000216a4, TOC0, void, CThing*, EPartType);
void CThing::RemovePart(EPartType type)
{
    if (type == PART_TYPE_MATERIAL_OVERRIDE && CustomThingData->PartMaterialOverride != NULL) 
    { 
        delete CustomThingData->PartMaterialOverride; 
        CustomThingData->PartMaterialOverride = NULL; 
    }

    CThing_RemovePart(this, type);
}

MH_DefineFunc(CThing_SetWorld, 0x0001f5ec, TOC0, void, CThing*, PWorld*, u32);
void CThing::SetWorld(PWorld* world, u32 preferred_uid)
{
    CThing_SetWorld(this, world, preferred_uid);
}

void CThing::InitializeExtraData()
{
    CustomThingData = new CCustomThingData();
}

void CThing::DestroyExtraData()
{
    if (CustomThingData != NULL)
    {
        if (CustomThingData->PartMaterialOverride != NULL)
            delete CustomThingData->PartMaterialOverride;

        delete CustomThingData;
    }
}