#include "resources/ResourceHooks.h"
#include "resources/ResourcePins.h"
#include "resources/ResourceOutfitList.h"
#include "resources/ResourceAnimatedTexture.h"
#include "customization/PoppetStyles.h"

#include "AlearSR.h"
#include "AlearHooks.h"

#include <Serialise.h>
#include <Variable.h>

#include <cell/DebugLog.h>
#include <ResourceDescriptor.h>
#include <ResourceSyncedProfile.h>
#include <ResourceLocalProfile.h>
#include <ResourceBigProfile.h>
#include <ResourceMaterial.h>
#include <ResourceSystem.h>
#include <ResourceGfxMaterial.h>
#include <ResourceGFXTexture.h>
#include <ResourceCharacterSettings.h>
#include <PartScriptName.h>
#include <PartPhysicsWorld.h>
#include <PartGeneratedMesh.h>
#include <ResourceGame.h>
#include <ResourceLevel.h>
#include <hook.h>
#include <ppcasm.h>

extern "C" uintptr_t _reflectextradata_load;
extern "C" uintptr_t _reflectextradata_save;
extern "C" uintptr_t _allocatenewresource_rtype_pins;
extern "C" uintptr_t _allocatenewresource_rtype_outfit_list;
extern "C" uintptr_t _allocatenewresource_rtype_animated_texture;
extern "C" uintptr_t _reflectresource_load_rtype_pins;
extern "C" uintptr_t _reflectresource_load_rtype_outfit_list;
extern "C" uintptr_t _reflectresource_load_rtype_animated_texture;
extern "C" uintptr_t _reflectresource_dependinate_ok;
extern "C" uintptr_t _get_serialisationtype_hook;
extern "C" uintptr_t _reflectresource_extra_load_rtype_synced_profile;
extern "C" uintptr_t _reflectresource_extra_save_rtype_synced_profile;
extern "C" uintptr_t _reflectresource_extra_fdepend_rtype_synced_profile;

extern "C" uintptr_t _initextradata_localprofile;
extern "C" uintptr_t _initextradata_syncedprofile;

extern "C" uintptr_t _radial_explosion_hook;
extern "C" uintptr_t _explosion_particle_and_sound_hook;

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;
#define ADD_ARRAY_ELEMENT(name, index) ret = Add(r, d.name[index], #name "_" #index); if (ret != REFLECT_OK) return ret;

void RMaterial::InitializeExtraData()
{
    ExplosionIgnoresPlayer = false;
    DisableExplosionCSG = false;
    DisableExplosionParticles = false;
    new (&ScorchMarkTexture) CP<RTexture>();

    // remember to deallocate this at some point
    new (&ExplosionSound) MMString<char>();
}

void RMaterial::DestroyExtraData()
{
    ScorchMarkTexture.~CP();
    ExplosionSound.~MMString();
}

void RPlan::InitializeExtraData()
{
    new (&TemplateLevel) CResourceDescriptor<RLevel>();
}

void RGfxMaterial::InitializeExtraData()
{
    new (&ParameterAnimations) CVector<CMaterialParameterAnimation>();
    new (&BoxAttributes) CVector<CMaterialBoxAttributes>();
    AlphaMode = 0;
}

void RGfxMaterial::DestroyExtraData()
{
    ParameterAnimations.~CVector();
}

void RCharacterSettings::InitializeExtraData()
{
    FramesTillFreeze = 120;
    ForceToFreeze = 300000.0f;
    ForceToShatterOnFreeze = 400000.0f;
    FramesTillMeltInWater = 150;
    FramesTillFrozenToDeath = 300;
}

void RLocalProfile::InitializeExtraData()
{
    new (&HiddenCategories) CVector<u32>();
    for (int i = 0; i < MAX_USER_EMOTES; ++i)
        new (&Emotes[i]) CResourceDescriptor<RPlan>();
    new (&SelectedAnimationStyle) CResourceDescriptor<RPlan>(CGUID(2507392567u));
    new (&PinsAwarded) CPinsAwarded();
}

void RSyncedProfile::InitializeExtraData()
{
    StyleFlags = STYLE_FLAGS_NONE;
    StyleID = STYLEID_DEFAULT;
    new (&AnimationStyle) MMString<char>();
    AnimationStyle = "sackboy";
}

template <typename R>
ReflectReturn OnSerializeExtraData(R& r, RMaterial& d)
{
    ReflectReturn ret = REFLECT_OK;

    if (r.GetSaving()) return REFLECT_NOT_IMPLEMENTED;
    if (r.GetCustomVersion() >= ALEAR_EXPLOSIVES)
    {
        ADD(ExplosionIgnoresPlayer);
        ADD(DisableExplosionCSG);
        ADD(DisableExplosionParticles);
        ADD(ScorchMarkTexture);
        ADD(ExplosionSound);
    }

    return ret;
}

template <typename R>
ReflectReturn OnSerializeExtraData(R& r, RSyncedProfile& d)
{
    DebugLog("Attempting to serialize additional information for RSyncedProfile (Revision=%08x, CustomRevision=%08x) (loading=%s, saving=%s)\n", r.GetRevision(), r.GetCustomVersion(), r.GetLoading() ? "true":"false", r.GetSaving() ? "true":"false");

    ReflectReturn ret = REFLECT_OK;
    if (r.GetCustomVersion() >= ALEAR_POPIT_STYLES)
    {
        ADD(StyleFlags);
        ADD(StyleID);
    }

    if (r.GetCustomVersion() >= ALEAR_ANIMATION_STYLES)
    {
        ADD(AnimationStyle);
    }

    return ret;
}

template <typename R>
ReflectReturn OnSerializeExtraData(R& r, RGfxMaterial& d)
{
    // We shouldn't ever be saving graphics materials from in-game anyway.
    if (!r.GetLoading()) return REFLECT_NOT_IMPLEMENTED;
    d.InitializeExtraData();

    if (r.GetCustomVersion() < ALEAR_PARAMETER_ANIMATIONS) return REFLECT_OK;

    ReflectReturn ret;

    if ((ret = Reflect(r, d.AlphaMode)) != REFLECT_OK) return ret;

    u32 count;
    if ((ret = Reflect(r, count)) != REFLECT_OK) return ret;
    if (!r.RequestToAllocate(count * sizeof(CMaterialParameterAnimation))) return REFLECT_EXCESSIVE_ALLOCATIONS;

    DebugLog("serializing %d parameter animations\n", count);

    d.ParameterAnimations.try_resize(count);
    for (int i = 0; i < count; ++i)
    {
        CMaterialParameterAnimation& anim = d.ParameterAnimations[i];

        if ((ret = Reflect(r, anim.BaseValue)) != REFLECT_OK) return ret;

        u32 num_keys;
        if ((ret = Reflect(r, num_keys)) != REFLECT_OK) return ret;
        if (!r.RequestToAllocate(num_keys * sizeof(float))) return REFLECT_EXCESSIVE_ALLOCATIONS;

        anim.Keys.try_resize(num_keys);
        if ((ret = r.ReadWrite((void*)anim.Keys.begin(), num_keys * sizeof(float))) != REFLECT_OK) return ret;


        // because of some dumb nonsense these get prefixed with array lengths,
        // despite being fixed size, should always be 3 though.
        u32 name_len;
        if ((ret = Reflect(r, name_len)) != REFLECT_OK) return ret;
        if (name_len != 3) return REFLECT_INVALID;
        if ((ret = r.ReadWrite((void*)anim.Name, 3)) != REFLECT_OK) return ret;
        if ((ret = Reflect(r, anim.ComponentsAnimated)) != REFLECT_OK) return ret;
    }

    if ((ret = Reflect(r, count)) != REFLECT_OK) return ret;
    if (!r.RequestToAllocate(count * sizeof(CMaterialBoxAttributes))) return REFLECT_EXCESSIVE_ALLOCATIONS;
    d.BoxAttributes.try_reserve(count);
    for (int i = 0; i < count; ++i)
    {
        CMaterialBoxAttributes& attr = d.BoxAttributes[i];

        if ((ret = Reflect(r, attr.SubType)) != REFLECT_OK) return ret;
        if ((ret = Reflect(r, attr.AnimIndex)) != REFLECT_OK) return ret;
        if ((ret = Reflect(r, attr.SecondaryAnimIndex)) != REFLECT_OK) return ret;
        if ((ret = Reflect(r, attr.ExtraParams[0])) != REFLECT_OK) return ret;
        if ((ret = Reflect(r, attr.ExtraParams[1])) != REFLECT_OK) return ret;
    }

    return ret;
}

template <typename R>
ReflectReturn OnSerializeExtraData(R& r, RLocalProfile& d)
{
    DebugLog("Attempting to serialize additional information for RLocalProfile[ALSR] (Revision=%08x, CustomRevision=%08x) (loading=%s, saving=%s)\n", r.GetRevision(), r.GetCustomVersion(), r.GetLoading() ? "true":"false", r.GetSaving() ? "true":"false");
    ReflectReturn ret = REFLECT_OK;
    
    if (r.GetCustomVersion() >= ALEAR_HIDDEN_CATEGORIES)
    {
        ADD(HiddenCategories);
    }

    if (r.GetCustomVersion() >= ALEAR_EMOTES)
    {
        ADD_ARRAY_ELEMENT(Emotes, 0);
        ADD_ARRAY_ELEMENT(Emotes, 1);
        ADD_ARRAY_ELEMENT(Emotes, 2);
        ADD_ARRAY_ELEMENT(Emotes, 3);
    }

    if (r.GetCustomVersion() >= ALEAR_ANIMATION_STYLES)
    {
        ADD(SelectedAnimationStyle);
    }

    if (r.GetCustomVersion() >= ALEAR_PINS)
    {
        ADD(PinsAwarded);
    }
    
    return ret;
}

template <typename R>
ReflectReturn OnSerializeExtraData(R& r, RCharacterSettings& d)
{
    ReflectReturn ret = REFLECT_OK;
    ADD(FramesTillFreeze);
    ADD(ForceToFreeze);
    ADD(ForceToShatterOnFreeze);
    ADD(FramesTillMeltInWater);
    ADD(FramesTillFrozenToDeath);
    return ret;
}

#undef ADD

bool ResourceHasCustomData(EResourceType type)
{
    return type == RTYPE_SYNCED_PROFILE || type == RTYPE_LOCAL_PROFILE || type == RTYPE_GFXMATERIAL || type == RTYPE_MATERIAL || type == RTYPE_SETTINGS_CHARACTER;
}

template <typename R>
ReflectReturn ReflectExtraResourceData(CResource* resource, R& r)
{
    if (!ResourceHasCustomData(resource->GetResourceType())) return REFLECT_OK;

    if (r.GetLoading())
    {
        switch (resource->GetResourceType())
        {
            case RTYPE_GFXMATERIAL:
            {
                ((RGfxMaterial*)resource)->InitializeExtraData();
                break;
            }
            case RTYPE_MATERIAL:
            {
                ((RMaterial*)resource)->InitializeExtraData();
                break;
            }
            case RTYPE_SETTINGS_CHARACTER:
            {
                ((RCharacterSettings*)resource)->InitializeExtraData();
                break;
            }
        }
    }

    // if there's nothing left to serialize, obviously doesn't have custom data

    bool is_compressed = true;
    u16 version = r.GetResourceVersion();
    u8 compression_flags = COMPRESS_INTS | COMPRESS_MATRICES;
    u32 branch_id = 0x4c425031;
    u32 branch_version = r.GetCustomVersion();

    ReflectReturn ret;

    if (!r.IsGatherVariables())
    {
        if (r.GetLoading() && r.GetVecLeft() == 0) return REFLECT_OK;

        if ((ret = r.Align(0x10)) != REFLECT_OK) return ret;
    
        u32 test_marker = 0x414c5352;
        if ((ret = Reflect(r, test_marker)) != REFLECT_OK) return ret;
    
        if (test_marker != 0x414c5352) return REFLECT_NOT_IMPLEMENTED;
    
        if ((ret = Reflect(r, version)) != REFLECT_OK) return ret;
        if (version >= ALEAR_LATEST_PLUS_ONE) return REFLECT_FORMAT_TOO_NEW;
    
        r.SetResourceVersion(version);
    
        if ((ret = Reflect(r, compression_flags)) != REFLECT_OK) return ret;
        if ((ret = Reflect(r, is_compressed)) != REFLECT_OK) return ret;
    
        if ((ret = Reflect(r, branch_id)) != REFLECT_OK) return ret;
        if (branch_id != 0x4c425031) return REFLECT_FORMAT_TOO_NEW;
    
        if ((ret = Reflect(r, branch_version)) != REFLECT_OK) return ret;
        if (branch_version >= ALEAR_BR1_LATEST_PLUS_ONE) return REFLECT_FORMAT_TOO_NEW;
        
        r.SetCustomVersion(branch_version);
    
        if (is_compressed)
        {
            if (r.GetLoading()) ret = r.LoadCompressionData(NULL);
            else if (r.GetSaving()) ret = r.StartCompressing();
    
            if (ret != REFLECT_OK) return ret;
        }
    
        r.SetCompressionFlags(compression_flags);
    }

    switch (resource->GetResourceType())
    {
        case RTYPE_SYNCED_PROFILE: ret = OnSerializeExtraData(r, *((RSyncedProfile*)resource)); break;
        case RTYPE_LOCAL_PROFILE: ret = OnSerializeExtraData(r, *((RLocalProfile*)resource)); break;
        case RTYPE_GFXMATERIAL: ret = OnSerializeExtraData(r, *((RGfxMaterial*)resource)); break;
        case RTYPE_MATERIAL: ret = OnSerializeExtraData(r, *((RMaterial*)resource)); break;
        case RTYPE_SETTINGS_CHARACTER: ret = OnSerializeExtraData(r, *((RCharacterSettings*)resource)); break;
    }

    if (!r.IsGatherVariables())
    {
        if (is_compressed)
        {
            if (r.GetLoading()) r.CleanupDecompression();
            else if (r.GetSaving()) r.FinishCompressing();
        }
    }

    return ret;
}

void HackSerializeExtraCharacterSettings(CGatherVariables& r, RCharacterSettings& d)
{
    if (r.GetLoading()) d.InitializeExtraData();
    OnSerializeExtraData(r, d);
}

template ReflectReturn OnSerializeExtraData<CReflectionLoadVector>(CReflectionLoadVector& r, RSyncedProfile& d);
template ReflectReturn OnSerializeExtraData<CReflectionSaveVector>(CReflectionSaveVector& r, RSyncedProfile& d);
template ReflectReturn OnSerializeExtraData<CReflectionFindDependencies>(CReflectionFindDependencies& r, RSyncedProfile& d);

template ReflectReturn ReflectExtraResourceData<CReflectionLoadVector>(CResource* resource, CReflectionLoadVector& r);
template ReflectReturn ReflectExtraResourceData<CReflectionSaveVector>(CResource* resource, CReflectionSaveVector& r);


RPins* AllocatePinsResource(EResourceFlag flags) 
{
    DebugLog("Attempting to allocate new RPins resource instance!\n"); 
    return new RPins(flags); 
}

ROutfitList* AllocateOutfitListResource(EResourceFlag flags)
{
    DebugLog("Attempting to allocate new ROutfitList resource instance!\n");
    return new ROutfitList(flags);
}

RAnimatedTexture* AllocateAnimatedTextureResource(EResourceFlag flags)
{
    DebugLog("Attempting to allocate new RAnimatedTexture resource instance!\n");
    return new RAnimatedTexture(flags);
}

void AttachResourceNames()
{
    MH_Poke32(0x000877c0, 0x2b830000 + (RTYPE_LAST - 1));

    static char* TABLE[RTYPE_LAST];
    for (int i = 0; i < RTYPE_LAST; ++i)
        TABLE[i] = "(unknown resource type)";
    
    // Copy the existing resource IDs into memory
    const int TABLE_ADDR = 0x009007fc;
    const int ID_COUNT = 0x2b;
    MH_Read(TABLE_ADDR, TABLE, ID_COUNT * sizeof(char*));
    
    TABLE[RTYPE_PINS] = "RPins";
    TABLE[RTYPE_OUTFIT_LIST] = "ROutfitList";
    TABLE[RTYPE_ANIMATED_TEXTURE] = "RAnimatedTexture";

    // Replace the TOC reference with our newly allocated table
    MH_Poke32(0x0091d4ac, (u32)TABLE);
}

void AttachResourceDependinateHooks()
{
    // We need to update the switch case that controls which
    // resource gets allocated based on the type.
    MH_Poke32(0x00725908, 0x2b800000 + (RTYPE_LAST - 1));

    // Initialise the switch table with the offsets to the invalid resource type case
    const int SWITCH_LABEL = 0x0072592c;
    const int NOP_LABEL = 0x007259d8;
    const int LABEL_COUNT = 0x2b;
    static s32 TABLE[RTYPE_LAST];
    for (int i = 0; i < RTYPE_LAST; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    TABLE[RTYPE_PINS] = (u32)&_reflectresource_dependinate_ok - (u32)TABLE;
    TABLE[RTYPE_OUTFIT_LIST] = (u32)&_reflectresource_dependinate_ok - (u32)TABLE;
    TABLE[RTYPE_ANIMATED_TEXTURE] = (u32)&_reflectresource_dependinate_ok - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0092e160, (u32)TABLE);
}

void AttachResourceLoadHooks()
{
    // We need to update the switch case that controls which
    // resource gets allocated based on the type.
    MH_Poke32(0x0072727c, 0x2b800000 + (RTYPE_LAST - 1));

    // Initialise the switch table with the offsets to the invalid resource type case
    const int SWITCH_LABEL = 0x007272a0;
    const int NOP_LABEL = 0x00727358;
    const int LABEL_COUNT = 0x2b;
    static s32 TABLE[RTYPE_LAST];
    for (int i = 0; i < RTYPE_LAST; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    TABLE[RTYPE_PINS] = (u32)&_reflectresource_load_rtype_pins - (u32)TABLE;
    TABLE[RTYPE_OUTFIT_LIST] = (u32)&_reflectresource_load_rtype_outfit_list - (u32)TABLE;
    TABLE[RTYPE_ANIMATED_TEXTURE] = (u32)&_reflectresource_load_rtype_animated_texture - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0092e164, (u32)TABLE);
}

void AttachResourceIds()
{
    MH_Poke32(0x000877ec, 0x2b830000 + (RTYPE_LAST - 1));

    static char* TABLE[RTYPE_LAST];

    // fairly sure this zero initialises by default, but just to be safe
    memset(TABLE, 0, sizeof(TABLE));

    // Copy the existing resource IDs into memory
    const int TABLE_ADDR = 0x00900750;
    const int ID_COUNT = 0x2b;
    MH_Read(TABLE_ADDR, TABLE, ID_COUNT * sizeof(char*));
    
    TABLE[RTYPE_PINS] = "PINb";
    TABLE[RTYPE_OUTFIT_LIST] = "OUTb";
    TABLE[RTYPE_ANIMATED_TEXTURE] = "ATXb";

    // Replace the TOC reference with our newly allocated table
    MH_Poke32(0x0091d4b0, (u32)TABLE);
}

void AttachResourceAllocationHooks()
{
    // We need to update the switch case that controls which
    // resource gets allocated based on the type.
    MH_Poke32(0x00088a98, 0x2b830000 + (RTYPE_LAST - 1));

    // Initialise the switch table with the offsets to the invalid resource type case
    const int SWITCH_LABEL = 0x00088ae4;
    const int NOP_LABEL = 0x00088b90;
    const int LABEL_COUNT = 0x2b;
    static s32 TABLE[RTYPE_LAST];
    for (int i = 0; i < RTYPE_LAST; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    TABLE[RTYPE_PINS] = (u32)&_allocatenewresource_rtype_pins - (u32)TABLE;
    TABLE[RTYPE_OUTFIT_LIST] = (u32)&_allocatenewresource_rtype_outfit_list - (u32)TABLE;
    TABLE[RTYPE_ANIMATED_TEXTURE] = (u32)&_allocatenewresource_rtype_animated_texture - (u32)TABLE;
    
    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0091d678, (u32)TABLE);
}

void AttachCustomRevisionHooks()
{
    // Store compression version in custom revision field of CReflectionLoadVector,
    // rather than just discarding it.
    // MH_Poke32(0x0058d5c8, ADDI(4, 23, 68));

    // Increase the size of certain resources to account for modifications
    MH_Poke32(0x00089260, LI(4, sizeof(RSyncedProfile)));
    MH_Poke32(0x00089590, LI(4, sizeof(RLocalProfile)));
    MH_Poke32(0x00088e44, LI(4, sizeof(RGfxMaterial)));
    MH_Poke32(0x00089000, LI(4, sizeof(RMaterial)));
    MH_Poke32(0x00088f48, LI(4, sizeof(RCharacterSettings)));

    // Increase the size of certain parts to account for modifications
    MH_Poke32(0x00020f30, LI(4, sizeof(PCreature)));
    MH_Poke32(0x0073ba14, LI(4, sizeof(PCreature)));

    // Some hooks to initialize extra data from resource constructors
    MH_PokeBranch(0x000ba1cc, &_initextradata_localprofile);
    MH_PokeBranch(0x000af44c, &_initextradata_syncedprofile);

    // Make sure to write our latest custom revision
    // MH_Poke32(0x0058ca18, LI(0, ALEAR_LATEST_PLUS_ONE - 1));
}

ESerialisationType GetPreferredSerialisationType(EResourceType type)
{
    switch (type)
    {
        case RTYPE_FILENAME:
        case RTYPE_FONTFACE:
            return PREFER_FILE;
        
        case RTYPE_GUID_SUBST:
        case RTYPE_SETTINGS_CHARACTER:
        case RTYPE_SETTINGS_SOFT_PHYS:
        case RTYPE_EDITOR_SETTINGS:
        case RTYPE_JOINT:
        case RTYPE_GAME_CONSTANTS:
        case RTYPE_POPPET_SETTINGS:
        case RTYPE_SETTINGS_NETWORK:
        case RTYPE_PARTICLE_SETTINGS:
        case RTYPE_PARTICLE_TEMPLATE:
        case RTYPE_PARTICLE_LIBRARY:
        case RTYPE_AUDIO_MATERIALS:
        case RTYPE_SETTINGS_FLUID:
        case RTYPE_TEXTURE_LIST:
        case RTYPE_MUSIC_SETTING:
        case RTYPE_MIXER_SETTINGS:
            return PREFER_TEXT;
        
        default: 
            return PREFER_BINARY;
    }
}

class CScopedPart {
public:
    inline CScopedPart(EPartType part) : Thing(NULL), Part(part) {}
    inline ~CScopedPart()
    {
        if (Thing != NULL)
            Thing->RemovePart(Part);
    }
    inline void SetThing(CThing* thing) { Thing = thing; }
private:
    CThing* Thing;
    EPartType Part;
};

#include <cell/DebugLog.h>


// have to include the thing as an argument since the fucking
// part hasnt been initialized
ReflectReturn PScriptName::LoadAlearData(CThing* thing)
{
    ReflectReturn ret = REFLECT_OK;
    CScopedPart _scoped_part(PART_TYPE_SCRIPT);

    const char* s = Name.c_str();
    int name_len = StringLength(s);
    int buf_len = Name.size();

    // Only delete the part on exit of this function if the
    // script doesn't actually have a name.
    if (name_len == 0) _scoped_part.SetThing(thing);

    // Custom data is stored after the null terminator,
    // if the length of the string is the same length
    // as the allocated buffer, there's nothing saved.
    if (name_len == buf_len) return ret;

    // Pre-emptively re-size the string buffer to remove
    // the extra data, this doesn't actually re-allocate the string,
    // so it's fine to do this before we process anything.
    Name.resize(name_len, '\0');

    // If there's not enough data for the header, just assume
    // it's some script with a malformed name.
    // 17 is the length of the header + the null byte of the script name
    if (name_len + 17 >= buf_len) return ret;

    int data_len = buf_len - name_len;
    CParasiticVector<char> vec((char*)(s + name_len + 1), data_len, data_len);
    CReflectionLoadVector r(&vec);

    DebugLog("addr: %08x, len: %08x\n", vec.begin(), vec.size());

    u32 magic, branch, version, flags;
    if ((ret = Reflect(r, magic)) != REFLECT_OK) return ret;
    if (magic != 0x414c5344 /* ALSD */) return REFLECT_INVALID;

    if ((ret = Reflect(r, branch)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, version)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, flags)) != REFLECT_OK) return ret;

    r.SetCompressionFlags(flags & 7);

    if (version >= ALEAR_BR1_LATEST_PLUS_ONE) return REFLECT_FORMAT_TOO_NEW;

    while (r.GetVecLeft() > 0)
    {
        u32 chunk;
        if ((ret = r.ReadWrite(&chunk, sizeof(u32))) != REFLECT_OK) return ret;

        DebugLog("processing chunk %08x\n", chunk);

        switch (chunk)
        {
            case 0x4746584D: /* GFXM */
            {
                PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
                if (mesh == NULL) return REFLECT_UNINITIALISED;

                if (version >= ALEAR_PARAMETER_ANIMATIONS)
                {
                    if ((ret = Reflect(r, mesh->TextureAnimationSpeed)) != REFLECT_OK) return ret;
                    if ((ret = Reflect(r, mesh->TextureAnimationSpeedOff)) != REFLECT_OK) return ret;
                }

                DebugLog("gfxm: <%f, %f>\n", mesh->TextureAnimationSpeed, mesh->TextureAnimationSpeedOff);
                break;
            }
            // case 0x4c414e44: /* LAND */
            // {
            //     PShape* shape = thing->GetPShape();
            //     if (shape == NULL) return REFLECT_UNINITIALISED;

            //     if (version >= ALEAR_SHAPE_BRIGHTNESS)
            //     {
            //         if ((ret = Reflect(r, shape->GetBrightness())) != REFLECT_OK) return ret;
            //         if ((ret = Reflect(r, shape->GetBrightnessOff())) != REFLECT_OK) return ret;

            //         v4 cached_original_color = shape->EditorColour;
            //         if ((ret = Reflect(r, cached_original_color)) != REFLECT_OK) return ret;
            //     }

            //     DebugLog("land: <%f, %f>\n", shape->GetBrightness(), shape->GetBrightnessOff());
            //     break;
            // }
            default:
            {
                return REFLECT_NOT_IMPLEMENTED;
            }
        }

        DebugLog("parsing next chunk... %d bytes remain\n", r.GetVecLeft());
    }

    return REFLECT_OK;
}

ReflectReturn PScriptName::WriteAlearData()
{
    ReflectReturn ret;
    ByteArray vec;
    CReflectionSaveVector r(&vec, 7);
    CThing* thing = GetThing();


    u32 magic = 0x414C5344, branch = 0x4c425031, version = ALEAR_BR1_LATEST_PLUS_ONE - 1, flags = COMPRESS_INTS;
    
    if ((ret = Reflect(r, magic)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, branch)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, version)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, flags)) != REFLECT_OK) return ret;

    r.SetCompressionFlags(flags & 7);

    PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
    if (mesh != NULL && mesh->HasCustomData())
    {
        u32 magic = 0x4746584D;

        // dont want to use compression for this
        if ((ret = r.ReadWrite(&magic, sizeof(u32))) != REFLECT_OK) return ret;

        if ((ret = Reflect(r, mesh->TextureAnimationSpeed)) != REFLECT_OK) return ret;
        if ((ret = Reflect(r, mesh->TextureAnimationSpeedOff)) != REFLECT_OK) return ret;
    }

    // PShape* shape = thing->GetPShape();
    // if (shape != NULL && shape->HasCustomData())
    // {
    //     u32 magic = 0x4c414e44;

    //     if ((ret = r.ReadWrite(&magic, sizeof(u32))) != REFLECT_OK) return ret;

    //     if ((ret = Reflect(r, shape->GetBrightness())) != REFLECT_OK) return ret;
    //     if ((ret = Reflect(r, shape->GetBrightnessOff())) != REFLECT_OK) return ret;

    //     v4 cached_original_color = shape->EditorColour;
    //     if ((ret = Reflect(r, cached_original_color)) != REFLECT_OK) return ret;
    // }

    int name_len = StringLength(Name.c_str());

    // remove any extra data from the string if it exists
    if (name_len != Name.size()) Name.resize(name_len, '\0');

    // Append our own data after the null terminator of the script name.
    Name.resize(name_len + vec.size(), '\0');
    memcpy(Name.begin() + name_len + 1, vec.begin(), vec.size());

    return REFLECT_OK;
}

bool CThing::HasCustomPartData()
{
    PGeneratedMesh* mesh = GetPGeneratedMesh();
    if (mesh != NULL && mesh->HasCustomData()) return true;
    // PShape* shape = GetPShape();
    // if (shape != NULL && shape->HasCustomData()) return true;

    return false;
}

void CThing::OnStartSave()
{
    // PWorld* world = GetPWorld();
    // if (world != NULL)
    // {
    //     for (CThing** it = world->Things.begin(); it != world->Things.end(); ++it)
    //     {
    //         CThing* thing = *it;
    //         if (thing == this || thing == NULL) continue;
    //         thing->OnStartSave();
    //     }
    // }

    if (!HasCustomPartData()) return;
    AddPart(PART_TYPE_SCRIPT_NAME);
    GetPScriptName()->WriteAlearData();
}

void CThing::OnFinishSave()
{
    // PWorld* world = GetPWorld();
    // if (world != NULL)
    // {
    //     for (CThing** it = world->Things.begin(); it != world->Things.end(); ++it)
    //     {
    //         CThing* thing = *it;
    //         if (thing == this || thing == NULL) continue;
    //         thing->OnFinishSave();
    //     }
    // }

    PScriptName* part = GetPScriptName();
    if (part != NULL)
    {
        const char* name = part->Name.c_str();
        int len = StringLength(name);

        if (len == 0) RemovePart(PART_TYPE_SCRIPT_NAME);
        else if (len != part->Name.size())
            part->Name.resize(len, '\0');
    }
}

#include "AlearConfig.h"
const u32 FALLBACK_GFX_MATERIAL_KEY = 66449u;
ReflectReturn CThing::OnLoad()
{
    if (gLoadDefaultMaterial)
    {
        PGeneratedMesh* mesh = GetPGeneratedMesh();
        if (mesh != NULL && !mesh->GfxMaterial)
        {
            DebugLog("Replacing thing[^%d]'s gfx material w/ fallback!!!\n", UID);
            mesh->GfxMaterial = LoadResourceByKey<RGfxMaterial>(FALLBACK_GFX_MATERIAL_KEY, 0, STREAM_PRIORITY_DEFAULT);
        }
    }

    if (gForceLoadEditable)
    {
        PShape* shape = GetPShape();
        if (shape != NULL)
        {
            shape->InteractEditMode |= 3;
            shape->InteractPlayMode |= 3;
        }

        PGroup* group = GetPGroup();
        if (group != NULL)
        {
            group->Copyright = false;
            group->Editable = true;
            group->PickupAllMembers = false;
        }
    }

    // temp temp remove this!!!
    // PShape* shape = GetPShape();
    // if (shape != NULL)
    // {
    //     float& brightness = shape->GetBrightness();
    //     float& brightness_off = shape->GetBrightnessOff();

    //     brightness = 1.0f;
    //     brightness_off = 0.0f;
    // }



    PScriptName* part = GetPScriptName();
    if (part != NULL) return part->LoadAlearData(this);
    return REFLECT_OK;
}

void PGeneratedMesh::InitializeExtraData()
{
    TextureAnimationSpeed = 1.0f;
    TextureAnimationSpeedOff = 0.0f;
}

void InitResourceHooks()
{
    MH_PokeBranch(0x006e00f8, &_hack_gather_character_settings_hook);

    AttachResourceAllocationHooks();
    AttachResourceDependinateHooks();
    AttachResourceLoadHooks();
    AttachResourceIds();
    AttachResourceNames();
    AttachCustomRevisionHooks();

    MH_PokeBranch(0x003c76bc, &_reflectextradata_load);
    MH_PokeBranch(0x003c7aac, &_reflectextradata_save);

    MH_PokeBranch(0x00211730, &_radial_explosion_hook);
    MH_PokeBranch(0x00211760, &_explosion_particle_and_sound_hook);

    MH_InitHook((void*)0x00087850, (void*)&GetPreferredSerialisationType);
    // MH_PokeBranch(0x00087850, &_get_serialisationtype_hook);

    // Switch the initialization of some fields in RGfxMaterial so our
    // booleans in the padded space get zero-initialized.
    MH_Poke32(0x000b4cd8, 0x93fd00e8 /* stw %r31, 0xe8(%r29) */);
    MH_Poke32(0x000b4cf4, 0x981d00e9 /* stb %r0, 0xe9(%r29) */);

    MH_PokeBranch(0x00717070, &_gmat_player_colour_hook);
    MH_PokeBranch(0x00770954, &_on_reflect_load_thing_hook);
    MH_PokeBranch(0x0076cf28, &_on_reflect_start_save_thing_hook);
    MH_PokeBranch(0x0076cf34, &_on_reflect_finish_save_thing_hook);
    MH_PokeBranch(0x00031f0c, &_initextradata_part_generatedmesh);
}