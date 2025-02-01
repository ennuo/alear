#ifndef SCRIPT_INSTANCE_H
#define SCRIPT_INSTANCE_H

#include <refcount.h>
#include <vector.h>

#include "vm/InstanceLayout.h"
#include "ResourceScript.h"

class CScriptInstance {
public:
    inline CScriptInstance() : Script(), InstanceLayout(), MemberVariables() {}
    inline CScriptInstance(CP<RScript>& script) : Script(script), InstanceLayout(), MemberVariables() {}
public:
    CP<RScript> Script;
    CP<CInstanceLayout> InstanceLayout;
    CRawVector<unsigned char> MemberVariables;
};



#endif // SCRIPT_INSTANCE_H