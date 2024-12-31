#include "resources/ResourceHooks.h"
#include "resources/ResourcePins.h"
#include "resources/ResourceOutfitList.h"
#include "resources/ResourceAnimatedTexture.h"
#include "customization/PoppetStyles.h"

#include "AlearSR.h"

#include <Serialise.h>
#include <Variable.h>

#include <cell/DebugLog.h>
#include <ResourceDescriptor.h>
#include <ResourceSyncedProfile.h>
#include <ResourceLocalProfile.h>
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

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;
#define ADD_ARRAY_ELEMENT(name, index) ret = Add(r, d.name[index], #name "_" #index); if (ret != REFLECT_OK) return ret;

void RLocalProfile::InitializeExtraData()
{
    new (&HiddenCategories) CVector<u32>();
    for (int i = 0; i < MAX_USER_EMOTES; ++i)
        new (&Emotes[i]) CResourceDescriptor<RPlan>();
    new (&SelectedAnimationStyle) CResourceDescriptor<RPlan>(CGUID(2507392567u));
}

void RSyncedProfile::InitializeExtraData()
{
    StyleFlags = STYLE_FLAGS_NONE;
    StyleID = STYLEID_DEFAULT;
    new (&AnimationStyle) MMString<char>();
    AnimationStyle = "sackboy";
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
    
    return ret;
}

#undef ADD

template <typename R>
ReflectReturn ReflectExtraResourceData(CResource* resource, R& r)
{
    // Realistically this is always no compression flags, but just in case
    u8 backup_compression_flags = r.GetCompressionFlags();

    if (r.GetCustomVersion() >= ALEAR_TEST_MARKER)
    {
        u32 test_marker = 0x414c5352;
        Reflect(r, test_marker);
    }

    if (r.GetCustomVersion() >= ALEAR_COMPRESSION_FLAGS)
    {
        u32 compression_flags = COMPRESS_INTS | COMPRESS_MATRICES;
        Reflect(r, compression_flags);
        r.SetCompressionFlags((u8)compression_flags);
    }

    switch (resource->GetResourceType())
    {
        case RTYPE_SYNCED_PROFILE: return OnSerializeExtraData(r, *((RSyncedProfile*)resource));
        case RTYPE_LOCAL_PROFILE: return OnSerializeExtraData(r, *((RLocalProfile*)resource));
    }

    r.SetCompressionFlags(backup_compression_flags);
    return REFLECT_OK;
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
    MH_Poke32(0x0058d5c8, ADDI(4, 23, 68));

    // Increase the size of certain resources to account for modifications
    MH_Poke32(0x00089260, LI(4, sizeof(RSyncedProfile)));
    MH_Poke32(0x00089590, LI(4, sizeof(RLocalProfile)));

    // Some hooks to initialize extra data from resource constructors
    MH_PokeBranch(0x000ba1cc, &_initextradata_localprofile);
    MH_PokeBranch(0x000af44c, &_initextradata_syncedprofile);

    // Make sure to write our latest custom revision
    MH_Poke32(0x0058ca18, LI(0, ALEAR_LATEST_PLUS_ONE - 1));
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

void InitResourceHooks()
{
    AttachResourceAllocationHooks();
    AttachResourceDependinateHooks();
    AttachResourceLoadHooks();
    AttachResourceIds();
    AttachResourceNames();
    AttachCustomRevisionHooks();

    MH_PokeBranch(0x003c76bc, &_reflectextradata_load);
    MH_PokeBranch(0x003c7aac, &_reflectextradata_save);

    MH_InitHook((void*)0x00087850, (void*)&GetPreferredSerialisationType);
    // MH_PokeBranch(0x00087850, &_get_serialisationtype_hook);
}