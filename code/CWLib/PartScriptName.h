#ifndef PART_SCRIPT_NAME_H
#define PART_SCRIPT_NAME_H

#include <MMString.h>
#include "SerialiseEnums.h"
#include "Part.h"

class PScriptName : public CPart {
public:
    ReflectReturn LoadAlearData(CThing* thing);
    ReflectReturn WriteAlearData();
public:
    MMString<char> Name;
};

#endif // PART_SCRIPT_NAME_H