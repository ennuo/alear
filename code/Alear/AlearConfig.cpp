#include <AlearConfig.h>

#include <ResourceGame.h>
#include <resources/ResourceAnimatedTexture.h>

#include <vm/NativeRegistry.h>
#include <vm/NativeFunctionCracker.h>

#include <ResourceSystem.h>
#include <MMString.h>
#include <ReadINI.h>
#include <vector.h>
#include <filepath.h>

void OnSetRenderDistanceToggle();

bool gPauseGameSim;

ConfigMap gConfigMap;
CConfigBool gUsePopitGradients(L"Popit", L"Interface - Use Popit Color Gradient", "PopitColorGradient", true);
CConfigBool gCanCollapseCategories(L"Popit", L"Interface - Can Collapse Categories", "CollapsibleCategories", true);
CConfigBool gCanHidePopit(L"Popit", L"Interface - Can Hide Tether & UI", "HideTetherAndUI", false);
CConfigBool gUseCustomCursors(L"Popit", L"Visual - Use Unique Cursor Sprites", "UniqueCursorSprites", true);
CConfigBool gColorCustomCursors(L"Popit", L"Visual - Use Player Color Cursor Sprites", "PlayerColorCursorSprites", false);
CConfigBool gAllowDebugTooltypes(L"Popit", L"Function - Allow Debug Tooltypes", "DebugToolTypes", true);
CConfigBool gAllowMeshScaling(L"Popit", L"Function - Allow Mesh Scaling", "AllowMeshScaling", false);
CConfigBool gAllowEyedroppingMeshes(L"Popit", L"Function - Allow Eyedropping Meshes", "AllowEyedroppingMeshes", false);

CConfigBool gDebugMaterialTweaks(L"Editor", L"Materials - Use Debug Tweak Options", "DebugTweakOptions", false);
CConfigBool gUseLegacyKeyColors(L"Editor", L"Gadgets - Use Legacy Key Colors", "LegacyKeyColors", true);
CConfigBool gUseNewKeyColorSelection(L"Editor", L"Gadgets - Use New Key Color Selection", "NewKeyColorSelection", true);

CConfigBool gResetLevelSettings(L"Loading", L"LevelSettings - Force Default", "ForceDefaultLevelSettings", false);
CConfigBool gForceLoadEditable(L"Loading", L"Shapes - Force Editable On Load", "ForceEditableOnLoad", false);
CConfigBool gUnlethalizeAllLethals(L"Loading", L"Shapes - Disable Lethals", "DisableShapeLethals", false);
CConfigBool gForceGFXShapes(L"Loading", L"Shapes - Turn into GFX", "ForceAttachShapeGraphics", false);
CConfigBool gPhysicsToGFX(L"Loading", L"Shapes - Visualize Physics", "VisualizeShapePhysics", false);
CConfigBool gForceMeshGFX(L"Loading", L"Objects - Turn into GFX", "ForceAttachObjectGraphics", false);
CConfigBool gLoadDefaultMaterial(L"Loading", L"Materials - Render Invisible GFX", "RenderInvisibleMaterials", false);
CConfigBool gForcePlainGFX(L"Loading", L"Materials - Use Plain GFX", "ForcePlainGraphicsMaterials", false);
CConfigBool gForcePlainBevels(L"Loading", L"Materials - Use Plain Bevels", "ForcePlainBevels", false);
CConfigBool gLoadRemoveAllStickers(L"Loading", L"Stickers - Remove all", "RemoveAllStickers", false);
CConfigBool gLoadRemoveAllDecorations(L"Loading", L"Decorations - Remove all", "RemoveAllDecorations", false);
CConfigBool gLoadRemoveAllLights(L"Loading", L"Lights -  Remove all", "RemoveAllLights", false);
CConfigBool gRemoveBurningScripts(L"Loading", L"Scripts - Remove Unused", "RemoveBurningScripts", true);
CConfigBool gFixupEmitters(L"Loading", L"Emitters - Fixup Invisible", "FixupInvisibleEmitters", false);
CConfigBool gUseAlternateJointMeshes(L"Loading", L"Joints - Use Alternate Meshes", "AlternateJointMeshes", true);

CConfigBool gPlayBackgroundStings(L"Audio", L"Audio - Play Background Stings", "EnableBackgroundStings", true);

CConfigFloat gRenderDistance(L"Render", L"Visual - Render Distance", "RenderDistance", gFarDist, 0.0f, NAN, 1000.0f, OnSetRenderDistanceToggle);

CConfigBool gUseDivergenceCheck(L"Gameplay", L"Divergence Check", "EnableDivergenceCheck", true);
CConfigBool gUseIceAccessibility(L"Gameplay", L"Ice Shake Mode", "IceShakeMode", false);

CConfigBool gEnablePodTransitionMask(L"Pod", L"Enable Pod Transition Mask", "EnablePodTransitionMask", true);

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

bool WantPodTransition()
{
    return gEnablePodTransitionMask;
}

void OnSetRenderDistanceToggle()
{
    gFarDist = gRenderDistance;
}

// CConfigBool gHideMSPF(L"Display", L"Hide MSPF Display", true);
tchar_t EMPTY_STRING[] = { 0x20 };

namespace alear
{
    void LoadConfig()
    {
        CFilePath fp(FPR_ALEAR, "config/gamevars.ini");
        CVector<MMString<char> > lines;
        if (!FileLoad(fp, lines)) return;

        char key[256] = {0};
        char val[256] = {0};
        char valspaces[256] = {0};
        char path[256] = {0};

        CVector<MMString<char> >::iterator itr = lines.begin();
        CVector<MMString<char> >::iterator end = lines.end();
        for (; itr != end; ++itr)
        {
            const char* s = itr->c_str();

            if (*s == '[') s = copyupto(path, s + 1, false);

            s = copyupto(key, s, false);

            *valspaces = '\0';
            if (*s == '=')
                s = copyupto(valspaces, s + 1, true);
            
            copyupto(val, valspaces, false);

            if (*key == '\0') continue;

            // typename ConfigMap::iterator category = gConfigMap.find(path);
            // if (category == gConfigMap.end())
            // {
            //     MMLog("config parser skipping %s because category %s does not exist\n", key, path);
            //     continue;
            // }


        }
    }

    void SaveConfig()
    {
        MMString<char> ini;
        char buf[256];
        for (ConfigMap::iterator it = gConfigMap.begin(); it != gConfigMap.end(); ++it)
        {
            ini += "[";
            WCharToMultiByteAppend(ini, it->first);
            ini += "]\n";

            CConfigOption* opt = it->second;
            while (opt)
            {
                ini += opt->GetIniName();
                ini += "=";

                switch (opt->GetType())
                {
                    case OPT_BOOL:
                    {
                        ini += (bool)*((CConfigBool*)opt) ? "true" : "false";
                        break;
                    }
                    case OPT_FLOAT:
                    {
                        sprintf(buf, "%f", (float)*((CConfigFloat*)opt));
                        ini += buf;
                        break;
                    }
                }

                ini += "\n";

                opt = opt->GetNext();
            }
        }

        int fd;
        if (FileOpen(CFilePath(FPR_ALEAR, "config/gamevars.ini"), fd, OPEN_WRITE))
        {
            FileWrite(fd, ini.begin(), ini.length());
            FileClose(fd);
        }
    }
}

#include <ppcasm.h>
extern "C" void _divergence_hook();
void AlearInitConf()
{
    MH_Poke32(0x000a2318, B(&_divergence_hook, 0x000a2318));
    // MH_Poke32(0x002daeac, BLR);
    MH_Poke32(0x002726fc, BLR);


    // gFarDist = 35000.0f;

    RegisterNativeFunction("Alear", ".set.EditMode__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetEditMode>);
    RegisterNativeFunction("Alear", ".get.EditMode__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetEditMode>);
    
    RegisterNativeFunction("Alear", ".set.DivergenceCheck__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetDivergenceCheckEnabled>);
    RegisterNativeFunction("Alear", ".set.PopitGradientEnabled__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetPopitGradientEnabled>);
    RegisterNativeFunction("Alear", ".get.PopitGradientEnabled__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetPopitGradientEnabled>);
    RegisterNativeFunction("Alear", ".get.DivergenceCheck__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetDivergenceCheckEnabled>);
    RegisterNativeFunction("Pod", ".get.WantPodTransition__", true, NVirtualMachine::CNativeFunction0<bool>::Call<WantPodTransition>);

    RegisterNativeFunction("Alear", "AddLevelListToInventory__i", true, NVirtualMachine::CNativeFunction1V<int>::Call<AddLevelListToInventory>);
    RegisterNativeFunction("Resource", "LoadAnimatedTextureByFilename__g", true, NVirtualMachine::CNativeFunction1<CP<CResource>, CGUID>::Call<LoadAnimatedTextureByFilename>);
}