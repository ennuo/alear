#ifndef AVATAR_H
#define AVATAR_H

#include "hack_thingptr.h"

class CAvatar {
protected:
    CThingPtr AvatarWorld;
};

class CPlatonicAvatar : public CAvatar {

};

#endif // AVATAR_H