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
CConfigBool gUsePopitGradients(L"Popit", L"Interface - Use Popit Color Gradient", true);
CConfigBool gCanCollapseCategories(L"Popit", L"Interface - Can Collapse Categories", true);
CConfigBool gCanHidePopit(L"Popit", L"Interface - Can Hide Tether & UI", false);
CConfigBool gUseCustomCursors(L"Popit", L"Visual - Use Unique Cursor Sprites", true);
CConfigBool gColorCustomCursors(L"Popit", L"Visual - Use Player Color Cursor Sprites", false);
CConfigBool gAllowDebugTooltypes(L"Popit", L"Function - Allow Debug Tooltypes", true);
CConfigBool gAllowMeshScaling(L"Popit", L"Function - Allow Mesh Scaling", false);
CConfigBool gAllowEyedroppingMeshes(L"Popit", L"Function - Allow Eyedropping Meshes", false);

CConfigBool gPlayBackgroundStings(L"Loading", L"Audio - Play Background Stings", true);
CConfigBool gUseAlternateJointMeshes(L"Loading", L"Visual - Use Alternate Joint Meshes", true);
CConfigBool gLoadDefaultMaterial(L"Loading", L"Visual - Use Fallback Graphics Materials", false);
CConfigBool gForceLoadEditable(L"Loading", L"Function - Force Editable On Load", false);

CConfigFloat gRenderDistance(L"Render", L"Visual - Render Distance", gFarDist, 0.0f, NAN, 1000.0f, OnSetRenderDistanceToggle);

CConfigBool gUseDivergenceCheck(L"Gameplay", L"Divergence Check", true);
CConfigBool gUseIceAccessibility(L"Gameplay", L"Ice Shake Mode", false);

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
        opt = opt->Next;
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