#ifndef VIEW_H
#define VIEW_H

#include "Camera.h"

class CView {
public:
    CCamera* Camera;
};

extern CView g_View;

#endif // VIEW_H