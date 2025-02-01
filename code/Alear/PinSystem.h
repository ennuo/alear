#ifndef PINS_H
#define PINS_H

#include <vector.h>
#include <GetLoginCookieTask.h>

class CGetPinsTask {
public:
    void StartTask();
    void Update();
private:
    ByteArray ResponseBuffer;
    CAuthenticatedTask AuthenticatedTask;
    float Timestamp;
    bool Valid;
};

void UpdatePinOverlay();
void RenderPinOverlay();

void InitPinHooks();
extern CGetPinsTask gPinsTask;

#endif // PINS_H