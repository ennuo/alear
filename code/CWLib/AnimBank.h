#ifndef ANIM_BANK_H
#define ANIM_BANK_H

#include <refcount.h>
#include <vector.h>

#include <Resource.h>

class RAnim : public CResource {};
class CAnimBank {
private:
    char Pad[0x50];
public:
    CVector<CP<RAnim> > Anim;
};

extern CAnimBank* gAnimBank;

#endif // ANIM_BANK_H