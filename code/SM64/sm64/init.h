#ifndef SM64_INIT_H
#define SM64_INIT_H

#include <libsm64.h>
#include <Camera.h>
#include "sm64/avatar.h"

bool InitMarioLib();
void CloseMarioLib();

void ClearMarioAvatars();
void UpdateMarioAvatars();
void UpdateMarioAvatarsRender();
void UpdateMarioDebugRender();
void SpawnMarioAvatar(EPadIndex pad, float x, float y, float z);
bool ApplyMainAvatarCamera(CCamera* camera);


#endif // SM64_INIT_H