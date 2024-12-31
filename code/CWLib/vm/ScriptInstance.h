#ifndef SCRIPT_INSTANCE_H
#define SCRIPT_INSTANCE_H

#include <refcount.h>
#include <vector.h>

#include "vm/InstanceLayout.h"
#include "ResourceScript.h"

class CScriptInstance {
private:
    CP<RScript> Script;
    CP<CInstanceLayout> InstanceLayout;
    CRawVector<unsigned char> MemberVariables;
};



#endif // SCRIPT_INSTANCE_H