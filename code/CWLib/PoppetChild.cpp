#include "PoppetChild.h"
#include "thing.h"

CPoppetChild::CPoppetChild() : Parent(NULL), PlayerThing()
{

}

CPoppetChild::~CPoppetChild()
{
    // if (Parent != NULL)
        // Parent->RemoveChild(this);
}

void CPoppetChild::SetParent(CPoppet* parent)
{
    Parent = parent;
}

void CPoppetChild::SetPlayer(CThing* player)
{
    PlayerThing = player;
}

PWorld* CPoppetChild::GetWorld()
{
    if (PlayerThing != NULL)
        return PlayerThing->World;
    return NULL;
}