#include "resources/ResourceHooks.h"
#include "resources/ResourcePins.h"
#include "customization/popitstyles.h"

#include "AlearSR.h"

#include <Serialise.h>
#include <Variable.h>

#include <cell/DebugLog.h>
#include <ResourceDescriptor.h>
#include <ResourceSyncedProfile.h>
#include <hook.h>
#include <ppcasm.h>

extern "C" uintptr_t _allocatenewresource_rtype_pins;
extern "C" uintptr_t _reflectresource_load_rtype_pins;
extern "C" uintptr_t _reflectresource_dependinate_ok;
extern "C" uintptr_t _get_serialisationtype_hook;
extern "C" uintptr_t _reflectresource_extra_load_rtype_synced_profile;
extern "C" uintptr_t _reflectresource_extra_save_rtype_synced_profile;
extern "C" uintptr_t _reflectresource_extra_fdepend_rtype_synced_profile;

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;
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
    else if (r.GetLoading())
    {
        d.StyleFlags = 0x0;
        d.StyleID = 0x0;
    }

    return ret;
}
#undef ADD

template ReflectReturn OnSerializeExtraData<CReflectionLoadVector>(CReflectionLoadVector& r, RSyncedProfile& d);
template ReflectReturn OnSerializeExtraData<CReflectionSaveVector>(CReflectionSaveVector& r, RSyncedProfile& d);
template ReflectReturn OnSerializeExtraData<CReflectionFindDependencies>(CReflectionFindDependencies& r, RSyncedProfile& d);

RPins* AllocatePinsResource(EResourceFlag flags) 
{
    DebugLog("Attempting to allocate new RPins resource instance!\n"); 
    return new RPins(flags); 
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

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0091d678, (u32)TABLE);
}

void AttachCustomRevisionHooks()
{
    // Store compression version in custom revision field of CReflectionLoadVector,
    // rather than just discarding it.
    MH_Poke32(0x0058d5c8, ADDI(4, 23, 68));

    // Increase the size of the RSyncedProfile allocation to accomodate our additions
    MH_Poke32(0x00089260, LI(4, sizeof(RSyncedProfile)));

    // Hook load function for RSyncedProfile to serialize additional data if the custom revision matches
    MH_PokeBranch(0x00727bf4, &_reflectresource_extra_load_rtype_synced_profile);
    MH_PokeBranch(0x0072c97c, &_reflectresource_extra_save_rtype_synced_profile);
    MH_PokeBranch(0x00725c9c, &_reflectresource_extra_fdepend_rtype_synced_profile);

    MH_Poke32(0x0058ca18, LI(0, ALEAR_LATEST_PLUS_ONE - 1));
}

void InitResourceHooks()
{
    AttachResourceAllocationHooks();
    AttachResourceDependinateHooks();
    AttachResourceLoadHooks();
    AttachResourceIds();
    AttachResourceNames();
    //AttachCustomRevisionHooks();

    MH_Poke32(0x00087850, B(&_get_serialisationtype_hook, 0x00087850));
}