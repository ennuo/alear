#include "alearconf.h"
#include "hook.h"
#include "ppcasm.h"

#include "ResourceGame.h"

#include "resources/ResourceAnimatedTexture.h"

#include "vm/NativeRegistry.h"
#include "vm/NativeFunctionCracker.h"

bool gUseDivergenceCheck = true;
bool gUsePopitGradients = true;
bool gHideMSPF = true;

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

StaticCP<RTranslationTable> gAlearTrans;

tchar_t EMPTY_STRING[] = { 0x20 };

bool CustomTryTranslate(u32 key, tchar_t const*& out)
{
    RTranslationTable* alear_trans = gAlearTrans;
    if (alear_trans != NULL && alear_trans->IsLoaded())
    {
        if (alear_trans->GetText(key, out))
            return true;
    }
    
    RTranslationTable* patch_trans = gPatchTrans;
    if (patch_trans != NULL && patch_trans->IsLoaded())
    {
        if (patch_trans->GetText(key, out))
            return true;
    }

    RTranslationTable* trans = gTranslationTable;
    if (trans != NULL && trans->IsLoaded())
    {
        if (trans->GetText(key, out))
            return true;
    }

    out = EMPTY_STRING;
    return false;
}

extern "C" void _divergence_hook();
void AlearInitConf()
{
    MH_Poke32(0x000a2318, B(&_divergence_hook, 0x000a2318));
    // MH_Poke32(0x002daeac, BLR);
    MH_Poke32(0x002726fc, BLR);

    MH_InitHook((void*)0x000232bc, (void*)&CustomTryTranslate);


    gFarDist = 35000.0f;

    

    RegisterNativeFunction("Alear", "SetEditMode__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetEditMode>);
    RegisterNativeFunction("Alear", "GetEditMode__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetEditMode>);
    
    RegisterNativeFunction("Alear", "SetDivergenceCheckEnabled__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetDivergenceCheckEnabled>);
    RegisterNativeFunction("Alear", "SetPopitGradientEnabled__b", true, NVirtualMachine::CNativeFunction1V<bool>::Call<SetPopitGradientEnabled>);
    RegisterNativeFunction("Alear", "GetPopitGradientEnabled__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetPopitGradientEnabled>);
    RegisterNativeFunction("Alear", "GetDivergenceCheckEnabled__", true, NVirtualMachine::CNativeFunction0<bool>::Call<GetDivergenceCheckEnabled>);
    RegisterNativeFunction("Alear", "AddLevelListToInventory__i", true, NVirtualMachine::CNativeFunction1V<int>::Call<AddLevelListToInventory>);
}