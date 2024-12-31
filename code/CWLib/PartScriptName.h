#ifndef PART_SCRIPT_NAME_H
#define PART_SCRIPT_NAME_H

#include <MMString.h>
#include "Part.h"

class PScriptName : public CPart {
public:
    MMString<char> Name;
};

#endif // PART_SCRIPT_NAME_H