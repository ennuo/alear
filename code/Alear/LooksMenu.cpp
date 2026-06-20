#include <Poppet.h>
#include <AlearShared.h>
#include <thing.h>
#include <padinput.h>
#include <Input.h>
#include <DebugLog.h>
#include <gooey/GooeyNodeManager.h>
#include <gfxcore.h>
#include <StringUtil.h>
#include <vm/NativeFunctionCracker.h>
#include <vm/VirtualMachine.h>
#include <Translate.h>
#include <Capture.h>
#include <ResourceSystem.h>

const u32 E_KEY_LOOKS_MENU = 3034081870ull;
static CSignature gRenderSig("Render__Q5Thing");

enum
{
    kRigMode_Translation,
    kRigMode_Rotation,
    kRigMode_Scale,
    kRigMode_Targets,
    kRigMode_Max
};

static float UnpackAnalogue(u16 raw)
{
    float val = (int)(raw - 128);
    if (abs(val) < 37.0f) return 0.0f;
    return val / 128.0f;
}

static const wchar_t* RIG_MODE_NAMES[] =
{
    L"Translation",
    L"Rotation",
    L"Scale",
    L"Morph Targets",
};

CLooksMenuState::CLooksMenuState() :
Mode(kRigMode_Scale), EditBone(0), EditMorph(0), UniformScaling(true)
{

}

CLooksMenuState::~CLooksMenuState()
{
    
}

void CLooksMenuState::Update()
{
    CThing* player = GetParent()->PlayerThing;
    if (player == NULL) return;

    PYellowHead* yellowhead = player->GetPYellowHead();
    PRenderMesh* render_mesh = player->GetPRenderMesh();
    if (render_mesh == NULL || !render_mesh->Mesh || !render_mesh->Mesh->IsLoaded()) 
        return;

    CInput* input = yellowhead->GetInput();
    if (input == NULL)
        return;
    
    const CMesh& mesh = render_mesh->Mesh->mesh;
    const int num_bones = mesh.Bones.size();
    const int num_morphs = mesh.MorphCount;

    if (input->IsJustClicked(BUTTON_CONFIG_LOOKSMENU_RESET))
    {
        switch (Mode)
        {
            case kRigMode_Translation: yellowhead->AnimBonePos[EditBone] = v4(0.0f); break;
            case kRigMode_Rotation: yellowhead->AnimBoneRot[EditBone] = q4::identity(); break;
            case kRigMode_Scale: yellowhead->AnimBoneScale[EditBone] = v4(1.0f); break;
            case kRigMode_Targets: yellowhead->AnimMorph[EditMorph]; break;
        }

        return;
    }

    bool changed_mode = false;
    bool changed_bone = false;

    if (input->IsJustClicked(BUTTON_CONFIG_LOOKSMENU_NEXT, Mode == kRigMode_Targets ? L"BP_NEXT_MORPH" : L"BP_NEXT_BONE"))
    {
        if (Mode == kRigMode_Targets)
            EditMorph = (EditMorph + 1) % num_morphs;
        else
            EditBone = (EditBone + 1) % num_bones;

        changed_bone = true;
    }

    if (input->IsJustClicked(BUTTON_CONFIG_LOOKSMENU_PREVIOUS, Mode == kRigMode_Targets ? L"BP_PREVIOUS_MORPH" : L"BP_PREVIOUS_BONE"))
    {
        if (Mode == kRigMode_Targets)
        {
            EditMorph = EditMorph - 1;
            if (EditMorph < 0) EditMorph = num_morphs - 1;
        }
        else
        {
            EditBone = EditBone - 1;
            if (EditBone < 0)
                EditBone = num_bones - 1;
        }

        changed_bone = true;
    }

    if (input->IsJustClicked(BUTTON_CONFIG_LOOKSMENU_CHANGE_MODE, L"BP_CHANGE_MODE"))
    {
        Mode = (Mode + 1) % kRigMode_Max;
        changed_mode = true;
    }
    
    if (Mode == kRigMode_Scale)
    {
        if (input->IsJustClicked(BUTTON_CONFIG_LOOKSMENU_CHANGE_SCALING_MODE, L"BP_CHANGE_MODE"))
            UniformScaling = !UniformScaling;
    }

    if (Mode == kRigMode_Scale && UniformScaling || Mode == kRigMode_Targets)
    {
        _SetButtonPrompt(BP_L2_R2, L"BP_SCALE", yellowhead->PlayerNumber);
    }
    else
    {
        _SetButtonPrompt(BP_LEFTSTICK, L"BP_XY", yellowhead->PlayerNumber);
        _SetButtonPrompt(BP_L2_R2, L"BP_Z", yellowhead->PlayerNumber);
    }

    if (changed_mode || changed_bone)
    {
        if (Mode == kRigMode_Rotation)
        {
            

        }
    }

    float FRAME_DELTA = 1.0f / 30.0f;
    if (Mode == kRigMode_Translation)
        FRAME_DELTA *= 100.0f;
    
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    u16 l2_pressure = input->GetPressureData(INPUT_ALL, PAD_PRESSURE_L2) & 0xff;
    if (l2_pressure < 10) l2_pressure = 0;
    u16 r2_pressure = input->GetPressureData(INPUT_ALL, PAD_PRESSURE_R2) & 0xff;
    if (r2_pressure < 10) r2_pressure = 0;

    float l2 = l2_pressure / 255.0f;
    float r2 = r2_pressure / 255.0f;

    v4 left_stick = input->GetStick(INPUT_ALL, STICK_LOOKSMENU) * FRAME_DELTA;
    x = left_stick.getX().getAsFloat();
    y = left_stick.getY().getAsFloat();
    z = (l2 - r2) * FRAME_DELTA;

    if (Mode == kRigMode_Targets)
    {
        yellowhead->AnimMorph[EditMorph] = SATURATE(yellowhead->AnimMorph[EditMorph] + z);
    }
    else
    {
        v4* out_v = NULL;
        switch (Mode)
        {
            case kRigMode_Translation: out_v = yellowhead->AnimBonePos + EditBone; break;
            case kRigMode_Rotation: out_v = (v4*)(yellowhead->AnimBoneRot + EditBone); break;
            case kRigMode_Scale: 
            {
                if (UniformScaling)
                {
                    x = z;
                    y = z;
                }

                out_v = yellowhead->AnimBoneScale + EditBone; 
                break;
            }
        }

        if (Mode == kRigMode_Rotation)
        {
            *(q4*)out_v *= q4::rotationX(x) * q4::rotationY(y) * q4::rotationZ(z);
        }
        else
        {
            *out_v += v4(x, y, z, 0.0f);
        }
    }

    if (input->IsJustClicked(BUTTON_CONFIG_LOOKSMENU_EXIT))
        GetParent()->PopMode();
}

void CLooksMenuState::Render()
{
    CPoppet* poppet = GetParent();
    CThing* player;
    if (poppet == NULL || (player = poppet->PlayerThing) == NULL) return;

    CScriptArguments args;
    args.AppendArg(player);

    NVirtualMachine::ExecuteStatic(player->World->GetThing(), E_KEY_LOOKS_MENU, gRenderSig, args, true, NULL);
}

MH_DefineFunc(SaveCostumeToInventory, 0x0038cf58, TOC1, void, CPoppetInventory*);
MH_DefineFunc(RemoveAllDecorationsFromMesh, 0x0005a748, TOC0, void, CThing*);
MH_DefineFunc(PStickers_RemoveAllDecals, 0x0005ad78, TOC0, void, PStickers*, int);

void PStickers::RemoveAllDecals(int index)
{
    PStickers_RemoveAllDecals(this, index);
}

void CPoppetInventory::SaveMorphToInventory()
{
    GetParent()->CaptureSubType = NCapture::SUBTYPE_COSTUME_MORPH;
    SaveCostumeToInventory(this);

    PStickers* stickers = CurrentSaveThing->GetPStickers();
    if (stickers != NULL)
    {
        stickers->RemoveAllDecals(-1);
        for (u32 i = 0; i < 14; ++i)
            stickers->RemoveAllDecals(i);
    }

    RemoveAllDecorationsFromMesh(CurrentSaveThing);

    PCostume* costume = CurrentSaveThing->GetPCostume();
    if (costume != NULL)
    {
        for (int i = 0; i < COSTUMEPART_COUNT; ++i)
            costume->Remove((ECostumePieceCategory)i);
        costume->GetNakedBaby();
        costume->SetMaterial(LoadResourceByKey<RGfxMaterial>(0x25e2), CResourceDescriptor<RPlan>(0x25e2));
    }
}

void SanitiseMorphForSave(CThing* thing)
{
    for (int i = 0; i < PART_TYPE_SIZE; ++i)
    {
        if (i == PART_TYPE_YELLOW_HEAD) continue;
        thing->RemovePart((EPartType) i);
    }
}

void SanitiseCostumeForSave(CThing* thing)
{
    CP<RMesh> mesh = thing->GetPRenderMesh()->Mesh;

    thing->RemovePart(PART_TYPE_CREATURE);
    thing->RemovePart(PART_TYPE_YELLOW_HEAD);
    thing->RemovePart(PART_TYPE_SCRIPT);
    thing->RemovePart(PART_TYPE_BODY);
    thing->RemovePart(PART_TYPE_SHAPE);
    thing->RemovePart(PART_TYPE_POS);
    thing->AddPart(PART_TYPE_RENDER_MESH);

    thing->GetPRenderMesh()->Mesh = mesh;
}

void OnSanitizeNewInventoryItem(CPoppetInventory* inventory)
{
    CThing* thing = inventory->CurrentSaveThing;

    if (inventory->GetParent()->CaptureSubType == NCapture::SUBTYPE_COSTUME_MORPH)
        SanitiseMorphForSave(thing);
    else if (thing->HasPart(PART_TYPE_COSTUME) && thing->HasPart(PART_TYPE_RENDER_MESH))
        SanitiseCostumeForSave(thing);
    
    inventory->GetParent()->CaptureSubType = NCapture::SUBTYPE_NONE;
}

void OnRefreshPlanDetails(CPlanDetails* details, CThing* thing)
{
    if (thing == NULL) return;

    u32 part_flags = 0;
    for (int i = 0; i < PART_TYPE_SIZE; ++i)
    {
        if (thing->GetPart((EPartType)i) != NULL)
            part_flags |= (1 << i);
    }

    // if we only have yellowhead, we're just a morph plan
    if (part_flags == (1 << PART_TYPE_YELLOW_HEAD))
    {
        details->Type = E_TYPE_COSTUME;
        details->SubType = E_SUBTYPE_MORPH;
        details->SetCategory(MakeLamsKeyID("TG_Morphs"));
        details->SetLocation(details->GetCategory());
    }
}

bool SetInventoryMorph(CPoppetInventory* inventory, CThing* global_thing, const RPlan* plan)
{
    CThing* player = inventory->GetPlayer();
    if (player == NULL) return false;

    PYellowHead* yellowhead = player->GetPYellowHead();
    if (yellowhead == NULL) return false;

    PYellowHead* saved_yellowhead = global_thing->GetPYellowHead();
    if (saved_yellowhead == NULL) return false;

    memcpy(yellowhead->AnimBonePos, saved_yellowhead->AnimBonePos, sizeof(yellowhead->AnimBonePos));
    memcpy(yellowhead->AnimBoneRot, saved_yellowhead->AnimBoneRot, sizeof(yellowhead->AnimBoneRot));
    memcpy(yellowhead->AnimBoneScale, saved_yellowhead->AnimBoneScale, sizeof(yellowhead->AnimBoneScale));
    memcpy(yellowhead->AnimMorph, saved_yellowhead->AnimMorph, sizeof(yellowhead->AnimMorph));

    return true;
}

namespace LooksMenuNativeFunctions
{
    const CMesh* GetMesh(CThing* thing)
    {
        PRenderMesh* part_render_mesh;
        if (thing == NULL || (part_render_mesh = thing->GetPRenderMesh()) == NULL) 
            return NULL;

        if (!part_render_mesh->Mesh || !part_render_mesh->Mesh->IsLoaded())
            return NULL;

        return &part_render_mesh->Mesh->mesh;
    }

    const CPoppet* GetPoppet(CThing* thing)
    {
        PYellowHead* part_yellowhead;
        if (thing == NULL || (part_yellowhead = thing->GetPYellowHead()) == NULL) 
            return NULL;
        return part_yellowhead->Poppet;
    }

    int GetNumBones(CThing* thing)
    {
        const CMesh* mesh = GetMesh(thing);
        return mesh != NULL ? mesh->Bones.size() : 0;
    }

    int GetNumMorphs(CThing* thing)
    {
        const CMesh* mesh = GetMesh(thing);
        return mesh != NULL ? mesh->MorphCount : 0;
    }

    const u32 MAX_MORPH_NAME = 16;
    const u32 MAX_BONE_NAME = 32;

    static wchar_t local_widechars[256];

    const wchar_t* GetBoneName(CThing* thing, int index)
    {
        char key[64];
        const tchar_t* text;
        sprintf(key, "LOOKS_MENU_SACKBOY_BONE_PRETTY_NAME_%d", index);
        if (TryTranslate(MakeLamsKeyID(key), text))
            return (const wchar_t*)text;

        *local_widechars = '\0';
        const CMesh* mesh = GetMesh(thing);
        if (mesh != NULL && index >= 0 && index < mesh->Bones.size())
        {
            const CBone& bone = mesh->Bones[index];
            for (u32 i = 0; i < MAX_BONE_NAME; ++i)
                local_widechars[i] = bone.Name[i];
        }

        return local_widechars;
    }

    const wchar_t* GetMorphName(CThing* thing, int index)
    {
        *local_widechars = '\0';
        const CMesh* mesh = GetMesh(thing);
        if (mesh != NULL && index >= 0 && index < mesh->MorphCount)
        {
            for (u32 i = 0; i < MAX_MORPH_NAME; ++i)
                local_widechars[i] = mesh->MorphNames[index][i];
        }

        return local_widechars;
    }

    int GetEditBone(CThing* thing)
    {
        const CPoppet* poppet = GetPoppet(thing);
        return poppet != NULL ? poppet->Looks.EditBone : 0;
    }

    int GetEditMorph(CThing* thing)
    {
        const CPoppet* poppet = GetPoppet(thing);
        return poppet != NULL ? poppet->Looks.EditMorph : 0;
    }

    int GetEditMode(CThing* thing)
    {
        const CPoppet* poppet = GetPoppet(thing);
        return poppet != NULL ? poppet->Looks.Mode : kRigMode_Translation;
    }

    bool GetUniformScaling(CThing* thing)
    {
        const CPoppet* poppet = GetPoppet(thing);
        return poppet != NULL ? poppet->Looks.UniformScaling : true;
    }
    
    const wchar_t* GetEditModeDisplayName(int mode)
    {
        if (mode < 0 || mode >= kRigMode_Max)
            return L"Invalid";
        return RIG_MODE_NAMES[mode];
    }

    const wchar_t* GetDisplayValue(CThing* thing, int component)
    {
        const CPoppet* poppet = GetPoppet(thing);
        if (poppet == NULL) return L"<?>";
        PYellowHead* yellowhead = thing->GetPYellowHead();

        char buf[256] = { 0 };

        if (poppet->Looks.Mode == kRigMode_Rotation)
        {
            return L"<?>";
        }
        else if (poppet->Looks.Mode != kRigMode_Targets)
        {
            v4* v = yellowhead->AnimBonePos + poppet->Looks.EditBone;
            if (poppet->Looks.Mode == kRigMode_Scale)
                v = yellowhead->AnimBoneScale + poppet->Looks.EditBone;
            sprintf(buf, "%f", v->getElem(component).getAsFloat());
        }
        else
        {
            sprintf(buf, "%f", yellowhead->AnimMorph[poppet->Looks.EditMorph]);   
        }

        for (u32 i = 0; i < 256; ++i)
            local_widechars[i] = buf[i];

        return local_widechars;
    }

    void Register()
    {
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetNumBones);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetNumMorphs);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetBoneName);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetMorphName);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetEditBone);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetEditMorph);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetEditMode);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetEditModeDisplayName);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetDisplayValue);
        REGISTER_NATIVE_FUNCTION_STATIC("LooksMenu", GetUniformScaling);
    }
}


extern "C" uintptr_t _popit_on_refresh_plan_details_hook;
extern "C" uintptr_t _popit_on_sanitize_inventory_item_hook;
extern "C" uintptr_t _on_calculate_input_mode_hook;
void AttachLooksMenuHooks()
{
    MH_PokeBranch(0x00097c78, &_popit_on_refresh_plan_details_hook);
    MH_PokeBranch(0x00388534, &_popit_on_sanitize_inventory_item_hook);
    MH_PokeBranch(0x00221f60, &_on_calculate_input_mode_hook);
}