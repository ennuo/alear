#include <refcount.h>
#include <cell/atomic.h>

CBaseCounted::~CBaseCounted()
{

}

int CBaseCounted::AddRef() 
{ 
    return cellAtomicIncr32((uint32_t*) &RefCount); 
}

int CBaseCounted::Release() 
{ 
    return cellAtomicDecr32((uint32_t*) &RefCount); 
}