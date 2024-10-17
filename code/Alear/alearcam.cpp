#include "alearcam.h"
#include "hook.h"
#include "ppcasm.h"

#include "gfxcore.h"
#include "padinput.h"

#include "cell/DebugLog.h"

#include "ResourceGFXFont.h"
#include "View.h"
#include "OverlayUI.h"
#include "LoadingScreen.h"

bool gShowCameraHelper = true;
bool gUseLegacyDebugCamera = false;
bool gDisableDOF = true;
bool gDisableFog = true;

void OnPredictionOrRenderUpdate()
{

}

float UnpackAnalogue(u16 raw)
{
    float val = (int)(raw - 128);
    if (abs(val) < 37.0f) return 0.0f;
    return val / 128.0f;
}

void HandleCameraMovementInput(CView* view)
{
    float lx = UnpackAnalogue(gPadData->LeftStickX);
    float ly = UnpackAnalogue(gPadData->LeftStickY);
    float rx = UnpackAnalogue(gPadData->RightStickX);
    float ry = UnpackAnalogue(gPadData->RightStickY);
    float l2 = gPadData->PressureData[PAD_PRESSURE_L2] / 255.0f;
    float r2 = gPadData->PressureData[PAD_PRESSURE_R2] / 255.0f;

    CDebugCamera& camera = view->DebugCamera;
    camera.Yaw += ry * 0.1f;
    camera.Pitch += rx * 0.1f;

    m44 inv_camrot = m44::rotationZYX(v3(-camera.Yaw, -camera.Pitch, 0.0f));

    v4 delta(lx * 100.0f, -ly * 100.0f, (l2 + -r2) * 100.0f, 1.0f);
    delta = inv_camrot * delta;

    camera.Pos += delta;
    camera.Foc = camera.Pos - (inv_camrot * v4(0.0f, 0.0f, 3000.0f, 1.0f));
}

void OnUpdateDebugCamera(CView* view)
{
    if (gUseLegacyDebugCamera || view->MaverickCam)
    {
        view->DebugCamera.Update(view->MaverickCam);
        view->DebugCamera.Apply(view->Camera);
        return;
    }

    HandleCameraMovementInput(view);
    view->DebugCamera.Apply(view->Camera);

    if ((gPadData->ButtonsDown & PAD_BUTTON_SQUARE) != 0)
    {
        gDisableDOF = !gDisableDOF;
    }

    if ((gPadData->ButtonsDown & PAD_BUTTON_CIRCLE) != 0)
    {
        gDisableFog = !gDisableFog;
    }

    if ((gPadData->ButtonsDown & PAD_BUTTON_TRIANGLE) != 0)
    {
        view->DebugCamera.Foc = view->DebugCamera.Pos - v4(0.0f, 0.0f, 3000.0f, 1.0f);
        view->DebugCamera.Yaw = 0.0f;
        view->DebugCamera.Pitch = 0.0f;
    }



    if (gDisableDOF)
    {
        gView.DofMax = 0.0f;
        gView.DofMin = 0.0f;
    }

    if (gDisableFog)
    {
        gView.FogMin = 0.0f;
        gView.FogMax = 0.0f;   
    }
}

const tchar_t* gTestString = (const tchar_t*)L"\uE001  Save Camera Shot";
void OnDrawPostComp(COverlayUI* interface)
{
    if (!gView.DebugCameraActive)
    {
        UpdateButtonPrompts();
        return;
    }

    if (!gShowCameraHelper) return;

    const u32 text_color = 0xFFFFFFFF;

    StartDrawText(true, NULL, NULL, false, true, gResX, gResY);


    // DrawText(gFont[FONT_DEFAULT], (const tchar_t*)L"\uE003  Hide UI", 16.0f, gResY - 21.0f, 0.0f, 15.0f, text_color, -1.0f);
    // DrawText(gFont[FONT_DEFAULT], gTestString, 16.0f, gResY - 21.0f, 0.0f, 15.0f, text_color, -1.0f);
    // DrawText(gFont[FONT_DEFAULT], (const tchar_t*)L"\uE003  Hide UI", 16.0f, gResY - 42.0f, 0.0f, 15.0f, text_color, -1.0f);


    //DrawText(gFont[FONT_DEFAULT], (const tchar_t*)L"DOF: Disabled", 16.0f, 21.0f, 0.0f, 7.0f, text_color, -1.0f);

    EndDrawText();
}

extern "C" void _alearcam_update_hook();
void InitCameraHooks()
{
    MH_Poke32(0x00014acc, B(&_alearcam_update_hook, 0x00014acc));
    MH_InitHook((void*)0x002716fc, (void*)&OnDrawPostComp);
    MH_InitHook((void*)0x001fc920, (void*)&OnUpdateDebugCamera);
}