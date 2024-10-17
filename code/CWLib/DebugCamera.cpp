#include "DebugCamera.h"
#include "hook.h"

MH_DefineFunc(CDebugCamera_Update, 0x001bd578, TOC0, void, CDebugCamera*, bool);
void CDebugCamera::Update(bool maverick)
{
    CDebugCamera_Update(this, maverick);
}

MH_DefineFunc(CDebugCamera_Apply, 0x001bf0b4, TOC0, void, CDebugCamera*, CCamera*);
void CDebugCamera::Apply(CCamera* camera)
{
    CDebugCamera_Apply(this, camera);
}