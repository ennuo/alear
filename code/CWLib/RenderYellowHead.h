#ifndef RENDER_YELLOW_HEAD_H
#define RENDER_YELLOW_HEAD_H

#include "SackBoyAnim.h"
class CRenderYellowHead {
private:
    char Pad[0x458];
public:
    CSackBoyAnim* SackBoyAnim;
};


#endif // RENDER_YELLOW_HEAD_H