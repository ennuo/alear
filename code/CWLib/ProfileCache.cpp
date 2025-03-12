#include <refcount.h>

#include "ResourceLocalProfile.h"

namespace ProfileCache
{
    MH_DefineFunc(GetOrCreateMainUserProfile, 0x0022d670, TOC0, CP<RLocalProfile>&);
};