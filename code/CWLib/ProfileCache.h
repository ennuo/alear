#ifndef PROFILE_CACHE_H
#define PROFILE_CACHE_H

#include <refcount.h>
class RLocalProfile;

namespace ProfileCache
{
    extern CP<RLocalProfile>& (*GetOrCreateMainUserProfile)();
};


#endif // PROFILE_CACHE_H