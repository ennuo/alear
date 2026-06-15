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
#include <mmalex.h>

void OnSetRenderDistanceToggle();

bool gPauseGameSim;

CConfigBool gEnablePodTransitionMask("Pod/Interface/Enable Pod Transition Mask", "EnablePodTransitionMask", false);

CConfigBool gUsePopitGradients("Popit/Interface/Use Popit Color Gradient", "PopitGradient", true);
CConfigBool gCanCollapseCategories("Popit/Interface/Can Collapse Categories", "CollapsibleCategories", true);
CConfigBool gSeparateToys("Popit/Interface/Separate Toys and Meshes", "SeparateToysAndMeshes", true);
CConfigBool gCanHidePopit("Popit/Interface/Can Hide Tether & UI", "EnableHideTether", false);
CConfigBool gUseCustomCursors("Popit/Visual/Use Unique Cursor Sprites", "UniqueCursorSprites", true);
CConfigBool gColorCustomCursors("Popit/Visual/Use Player Color Cursor Sprites", "ColourCursorSprites", false);
CConfigBool gAllowDebugTooltypes("Popit/Function/Allow Debug Tooltypes", "DebugTooltypes", true);
CConfigBool gAllowMeshScaling("Popit/Function/Allow Mesh Scaling", "AllowMeshScaling", false);
CConfigBool gAllowEyedroppingMeshes("Popit/Function/Allow Eyedropping Meshes", "AllowEyedroppingMeshes", false);

CConfigBool gDebugMaterialTweaks("Editor/Materials/Use Debug Tweak Options", "DebugTweakOptions", false);
CConfigBool gUseLegacyKeyColors("Editor/Gadgets/Use Legacy Key Colors", "LegacyKeyColors", true);
CConfigBool gUseNewKeyColorSelection("Editor/Gadgets/Use New Key Color Selection", "NewKeyColorSelection", true);

CConfigBool gResetLevelSettings("Loading/LevelSettings/Force Default", "ForceDefaultLevelSettings", false);
CConfigBool gForceLoadEditable("Loading/Shapes/Force Editable On Load", "ForceShapesEditableOnLoad", false);
CConfigBool gUnlethalizeAllLethals("Loading/Shapes/Disable Lethals", "DisableShapeLethalsOnLoad", false);
CConfigBool gForceGFXShapes("Loading/Shapes/Turn into GFX", NULL, false);
CConfigBool gPhysicsToGFX("Loading/Shapes/Visualize Physics", NULL, false);
CConfigBool gForceMeshGFX("Loading/Objects/Turn into GFX", NULL, false);
CConfigBool gLoadDefaultMaterial("Loading/Materials/Render Invisible GFX", NULL, false);
CConfigBool gForcePlainGFX("Loading/Materials/Use Plain GFX", NULL, false);
CConfigBool gForcePlainBevels("Loading/Materials/Use Plain Bevels", NULL, false);
CConfigBool gLoadRemoveAllStickers("Loading/Stickers/Remove all", "RemoveAllStickersOnLoad", false);
CConfigBool gLoadRemoveAllDecorations("Loading/Decorations/Remove all", "RemoveAllDecorationsOnLoad", false);
CConfigBool gLoadRemoveAllLights("Loading/Lights/Remove all", "RemoveAllLightsOnLoad", false);
CConfigBool gRemoveBurningScripts("Loading/Scripts/Remove Unused", "RemoveUnusedScriptsOnLoad", true);
CConfigBool gFixupEmitters("Loading/Emitters/Fixup Invisible", "FixupInvisibleEmitters", false);
CConfigBool gUseAlternateJointMeshes("Loading/Joints/Use Alternate Meshes", "AlternateJointMeshes", true);

CConfigBool gPlayBackgroundStings("Audio/Play Background Stings", "EnableBackgroundStings", true);

CConfigFloat gRenderDistance("Render/Visual/Render Distance", "RenderDistance", gFarDist, 0.0f, NAN, 1000.0f, OnSetRenderDistanceToggle);

CConfigBool gUseDivergenceCheck("Gameplay/Divergence Check", "DivergenceCheck", true);
CConfigBool gUseIceAccessibility("Gameplay/Ice Shake Mode", "IceShakeMode", false);


void ToggleEditOrPlayMode()
{
    gGame->EditMode = !gGame->EditMode;
}

static CConfigFunction gSaveConfig("Save Config", alear::SaveConfig);
static CConfigFunction gToggleEditOrPlayMode("Toggle Edit or Play Mode", ToggleEditOrPlayMode);

static CConfigOption* gConfigHead;
CConfigFolder gConfigRoot[128];
static u8 gConfigAllocator = 1;

void CConfigOption::AddToRegistry()
{
    Next = gConfigHead;
    gConfigHead = this;
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

CConfigFolder::CConfigFolder() : Path(), DisplayName(), Options(),
NextSibling(), FirstChild(), Parent(), Open()
{

}

CConfigFolder* FindOrCreateFolder(const TextRange<char>& name, CConfigFolder& parent = *gConfigRoot)
{
    u8 child = parent.FirstChild;
    while (child != 0)
    {
        CConfigFolder& f = gConfigRoot[child];
        if (f.Path.Equals(name))
            return &f;
        child = f.NextSibling;
    }

    u8 i = gConfigAllocator++;
    CConfigFolder& f = gConfigRoot[i];
    f.Path = name;
    f.Parent = &parent - gConfigRoot;
    MultiByteToWChar(f.DisplayName, name.Begin, name.End);
    f.NextSibling = parent.FirstChild;
    parent.FirstChild = i;
    
    return &f;
}

namespace alear
{
    void SetupConfigTables()
    {
        gConfigRoot->Open = true;
        for (CConfigOption* opt = gConfigHead; opt != NULL; opt = opt->GetNext())
        {
            CConfigFolder* parent = gConfigRoot;

            const char* begin = opt->GetPath();
            const char* end = NULL;

            while ((end = strchr(begin, '/')) != NULL)
            {
                parent = FindOrCreateFolder(TextRange<char>(begin, end), *parent);
                begin = end + 1;
            }

            opt->Sibling = parent->Options;
            parent->Options = opt;
            MultiByteToWChar(opt->DisplayName, begin, NULL);
        }
    }

    CConfigOption* FindConfigOption(const char* ini_file_name)
    {
        for (CConfigOption* opt = gConfigHead; opt != NULL; opt = opt->GetNext())
        {
            const char* opt_name = opt->GetIniFileName();
            if (opt_name == NULL) continue;

            if (StringCompare(ini_file_name, opt_name) == 0)
                return opt;
        }

        return NULL;
    }

    void LoadConfig()
    {
        SetupConfigTables();

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

            s = copyupto(key, s, false);

            *valspaces = '\0';
            if (*s == '=')
                s = copyupto(valspaces, s + 1, true);
            
            copyupto(val, valspaces, false);

            if (*key == '\0') continue;

            CConfigOption* opt = FindConfigOption(key);
            if (opt != NULL)
            {
                switch (opt->GetType())
                {
                    case OPT_BOOL:
                        *((CConfigBool*)opt) = StringCompare(val, "true") == 0;
                        break;
                    case OPT_FLOAT:
                        *((CConfigFloat*)opt) = mmalex::strtod(val, NULL);
                        break;
                }
            }
        }
    }

    void SaveConfig()
    {
        MMString<char> ini;
        char buf[256];
        for (CConfigOption* opt = gConfigHead; opt != NULL; opt = opt->GetNext())
        {
            if (opt->GetType() == OPT_FUNCTION || opt->GetIniFileName() == NULL) continue;

            ini += opt->GetIniFileName();
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