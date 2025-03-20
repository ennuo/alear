#include "AlearConfig.h"
#include "hook.h"
#include "ppcasm.h"

#include "ResourceGame.h"

#include "resources/ResourceAnimatedTexture.h"

#include "vm/NativeRegistry.h"
#include "vm/NativeFunctionCracker.h"

#include <ResourceSystem.h>

void OnSetRenderDistanceToggle();

bool gPauseGameSim;

ConfigMap gConfigMap;
CConfigBool gUsePopitGradients(L"Popit", L"Gradient", true);
CConfigBool gUseDivergenceCheck(L"Game", L"Divergence Check", true);
CConfigFloat gRenderDistance(L"Render", L"Render Distance", gFarDist, 0.0f, NAN, 1000.0f, OnSetRenderDistanceToggle);

CConfigBool gLoadDefaultMaterial(L"Load", L"Use Fallback Graphics Materials", false);
CConfigBool gForceLoadEditable(L"Load", L"Force Editable On Load", false);

CConfigBool gForceMeshScaling(L"Debug", L"Force Mesh Scaling", false);

CConfigBool gUseIceAccessibility(L"Accessibility", L"Ice Shake Mode", false);

void CConfigOption::AddToRegistry()
{
    typename ConfigMap::iterator it = gConfigMap.find(Category);
    if (it == gConfigMap.end())
    {
        gConfigMap.insert(ConfigMap::value_type(Category, this));
        return;
    }

    CConfigOption* opt = it->second;
    while (opt->Next != NULL)
        opt->Next = opt->Next;
    opt->Next = this;
}

bool GetEditMode() { return gGame->EditMode; }
void SetEditMode(bool edit_mode) { gGame->EditMode = edit_mode; }
bool GetDivergenceCheckEnabled() { return gUseDivergenceCheck; }
bool GetPopitGradientEnabled() { return gUsePopitGradients; }

void SetDivergenceCheckEnabled(bool enabled) { gUseDivergenceCheck = enabled; }
void SetPopitGradientEnabled(bool enabled) { gUsePopitGradients = enabled; }

MH_DefineFunc(Debug_DoInventoryStuff, 0x000b9bc0, TOC0, void, int);
void AddLevelListToInventory(int guid)
{
    Debug_DoInventoryStuff(guid);
}

CP<CResource> LoadAnimatedTextureByFilename(CGUID guid)
{
    return (CP<CResource>)LoadResourceByKey<RAnimatedTexture>(guid.guid, 0, STREAM_PRIORITY_DEFAULT);
}

void OnSetRenderDistanceToggle()
{
    gFarDist = gRenderDistance;
}

// CConfigBool gHideMSPF(L"Display", L"Hide MSPF Display", true);
tchar_t EMPTY_STRING[] = { 0x20 };

extern "C" void _divergence_hook();
void AlearInitConf()
{
    MH_Poke32(0x000a2318, B(&_divergence_hook, 0x000a2318));
    // MH_Poke32(0x002daeac, BLR);
    MH_Poke32(0x002726fc, BLR);


    // gFarDist = 35000.0f;

    RegisterNativeFunction("Alear", "SetEditMode__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetEditMode>);
    RegisterNativeFunction("Alear", "GetEditMode__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetEditMode>);
    
    RegisterNativeFunction("Alear", "SetDivergenceCheckEnabled__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetDivergenceCheckEnabled>);
    RegisterNativeFunction("Alear", "SetPopitGradientEnabled__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetPopitGradientEnabled>);
    RegisterNativeFunction("Alear", "GetPopitGradientEnabled__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetPopitGradientEnabled>);
    RegisterNativeFunction("Alear", "GetDivergenceCheckEnabled__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetDivergenceCheckEnabled>);
    RegisterNativeFunction("Alear", "AddLevelListToInventory__i", true, NVirtualMachine::CNativeFunction1V<int>::Call<AddLevelListToInventory>);

    RegisterNativeFunction("Resource", "LoadAnimatedTextureByFilename__g", true, NVirtualMachine::CNativeFunction1<CP<CResource>, CGUID>::Call<LoadAnimatedTextureByFilename>);
}