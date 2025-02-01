#ifndef SCRIPT_OBJECT_RESOURCE_H
#define SCRIPT_OBJECT_RESOURCE_H

#include <refcount.h>
#include <Resource.h>

#include "vm/ScriptObject.h"

class CScriptObjectResource : public CScriptObject {
public:
    inline const CP<CResource>& GetResource() { return Resource; }
private:
    CP<CResource> Resource;
    bool Canonical;
};

#endif // SCRIPT_OBJECT_RESOURCE_H