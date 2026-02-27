#include "TweakShape.h"

#include <thing.h>
#include <refcount.h>
#include <Poppet.h>
#include <ResourceSystem.h>
#include <ResourceScript.h>
#include <ResourceGFXMesh.h>
#include <PartGeneratedMesh.h>
#include <PartRenderMesh.h>
#include <ResourceTranslationTable.h>
#include <PartScript.h>

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

#include <poppet/ScriptObjectPoppet.h>

#include "CheckpointStyles.h"
#include "ExplosiveStyles.h"
#include "MiscMeshStyles.h"

const u32 E_TWEAK_SHAPE_SCRIPT = 3796510132u;
const u32 E_TWEAK_CHECKPOINT_SCRIPT = 4286805021u;
const u32 E_TWEAK_EXPLOSIVE_SCRIPT = 3549987402u;
const u32 E_TWEAK_MAGIC_EYE_SCRIPT = 34681;
const u32 E_TWEAK_SPIKE_PLATE_SCRIPT = 34681;

const u32 E_LAMS_TWEAKABLE_MATERIAL = MakeLamsKeyID("TWEAKABLE_MATERIAL", "_NAME");
const u32 E_LAMS_TWEAKABLE_MESH = MakeLamsKeyID("TWEAKABLE_MESH", "_NAME");
const u32 E_LAMS_TWEAKABLE_DECAL = MakeLamsKeyID("TWEAKABLE_DECAL", "_NAME");

StaticCP<RScript> gTweakShapeScript;
StaticCP<RScript> gTweakCheckpointScript;
StaticCP<RScript> gTweakExplosiveScript;
StaticCP<RScript> gTweakMagicEyeScript;
StaticCP<RScript> gTweakSpikePlateScript;

struct SCheckpointStyle {
    CGUID Mesh[NUM_CHECKPOINT_TYPES];
    u32 SoundEnum;
};

SCheckpointStyle gCheckpointStyles[] =
{
    // Cardboard
    {
        { 122175, 120921, 120915, 120935 },
        6
    },

    // Wood
    {
        { 31238, 11668, 55455, 68386 },
        3
    },

    // Plastic
    {
        { 119228, 119230, 119227, 120939 },
        24
    },

    // Chrome
    {
        { 119221, 119217, 119220, 120941 },
        32
    }
};

struct SExplosiveStyle {
    CGUID Mesh[NUM_EXPLOSIVE_TYPES];
    CGUID Material[NUM_EXPLOSIVE_TYPES];
};

SExplosiveStyle gExplosiveStyles[] =
{
    // Default
    {
        { 34064, 34066 },
        { 35546, 35545 }
    },

    // Shock
    {
        { 3039519250u, 3211308833u },
        { 3970373057u, 2644769124u }
    },

    // Freeze
    {
        { 2361610045u, 2252254037u },
        { 4277027661u, 2417976744u }
    },

    // Stun
    {
        { 4216187055u, 3090086945u },
        { 2924524564u, 4267594693u }
    },
    
    // Launch
    {
        { 2303256371u, 3063072232u },
        { 2476861601u, 3052190506u }
    },

    // Add
    {
        { 2303256371u, 3063072232u },
        { 3982758946u, 2207593159u }
    }
};

CGUID gLevelKeyStyles[] =
{
    // Gemstones
    3763,
    // Brass
    44681,
};

CGUID gMagicEyeStyles[] =
{
    // Normal
    31054,
    // Cute
    31054,
    // Evil
    31054
};

struct SCreatureBrainStyle {
    CGUID Mesh;
    CGUID Material;
    u32 SoundEnum;
};

SCreatureBrainStyle gCreatureBrainStyles[] =
{
    // Protected
    {
        39574,
        10724,
        32
    },
    // Unprotected
    {
        38925,
        17661,
        0
    }
};

CGUID gMagicMouthStyles[] =
{
    // Normal
    22542
};

struct SLeverSwitchStyle {
    CGUID Mesh[NUM_LEVER_SWITCH_TYPES];
    u32 SoundEnum;
};

SLeverSwitchStyle gLeverSwitchStyles[] =
{
    // Cardboard
    {
        { 21936, 21940 },
        6
    },
    // Wood
    {
        { 21935, 21939 },
        3
    },
    // Plastic
    {
        { 21935, 21939 },
        24
    },
    // Chrome
    {
        { 21935, 21939 },
        32
    }
};

struct SBouncePadStyle {
    CGUID Mesh;
    u32 SoundEnum;
};

SBouncePadStyle gBouncePadStyles[] =
{
    // Cardboard
    {
        77780,
        6
    },
    // Wood
    {
        77780,
        3
    },
    // Plastic
    {
        77780,
        24
    },
    // Chrome
    {
        77780,
        32
    }
};

struct SSpikePlateStyle {
    CGUID Mesh[NUM_SPIKE_PLATE_TYPES];
    u32 SoundEnum;
};

SSpikePlateStyle gSpikePlateStyles[] =
{
    // Cardboard
    {
        { 29972, 29975 },
        6
    },
    // Wood
    {
        { 29972, 29975 },
        3
    },
    // Plastic
    {
        { 29972, 29975 },
        24
    },
    // Chrome
    {
        { 29972, 29975 },
        32
    }
};

CGUID GetMeshGUID(CThing* thing)
{
    if (thing == NULL) return 0;
    PRenderMesh* part = thing->GetPRenderMesh();
    if (part == NULL) return 0;
    CP<RMesh>& mesh = part->Mesh;
    if (!mesh) return 0;
    return mesh->GetGUID();
}

s32 GetCheckpointType(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gCheckpointStyles); ++i)
    for (int j = 0; j < NUM_CHECKPOINT_TYPES; ++j)
    {
        if (gCheckpointStyles[i].Mesh[j] == guid)
            return j;
    }

    return CHECKPOINT_SINGLE;
}

PCheckpoint* FindPartCheckpoint(CThing* thing)
{
    if (thing == NULL) return NULL;
    PCheckpoint* checkpoint = thing->GetPCheckpoint();
    if (checkpoint != NULL) return checkpoint;

    thing = thing->FirstChild;
    while (thing != NULL)
    {
        checkpoint = thing->GetPCheckpoint();
        if (checkpoint != NULL)
            return checkpoint;
        thing = thing->NextSibling;
    }

    return NULL;
}

void SetCheckpointStyle(CThing* thing, s32 type_index, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    SCheckpointStyle& style = gCheckpointStyles[style_index];
    u32 mesh_key = style.Mesh[type_index];
    u32 sound_enum = style.SoundEnum;

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
    
    PShape* shape = thing->GetPShape();
    if (shape != NULL)
        shape->SoundEnumOverride = sound_enum;

    PCheckpoint* checkpoint = FindPartCheckpoint(thing);
    if (checkpoint == NULL) return;

    switch (type_index)
    {
        case CHECKPOINT_ENTRANCE:
        case CHECKPOINT_SINGLE:
            checkpoint->InstanceInfiniteSpawns = false;
            checkpoint->LifeMultiplier = 1;
            break;
        case CHECKPOINT_DOUBLE:
            checkpoint->LifeMultiplier = 2;
            checkpoint->InstanceInfiniteSpawns = false;
            break;
        case CHECKPOINT_INFINITE:
            checkpoint->LifeMultiplier = 1;
            checkpoint->InstanceInfiniteSpawns = true;
            break;
    }
}

s32 GetCheckpointStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gCheckpointStyles); ++i)
    for (int j = 0; j < NUM_CHECKPOINT_STYLES; ++j)
    {
        if (gCheckpointStyles[i].Mesh[j] == guid)
            return i;
    }

    return 0;
}

bool IsCheckpointMesh(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return false;

    for (int i = 0; i < ARRAY_LENGTH(gCheckpointStyles); ++i)
    for (int j = 0; j < NUM_CHECKPOINT_TYPES; ++j)
    {
        if (gCheckpointStyles[i].Mesh[j] == guid)
            return true;
    }

    return false;
}

s32 GetExplosiveType(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gExplosiveStyles); ++i)
    for (int j = 0; j < NUM_EXPLOSIVE_TYPES; ++j)
    {
        if (gExplosiveStyles[i].Mesh[j] == guid)
            return j;
    }

    return EXPLOSIVE_BANGER;
}

void SetExplosiveStyle(CThing* thing, s32 type_index, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    SExplosiveStyle& style = gExplosiveStyles[style_index];
    u32 mesh_key = style.Mesh[type_index];
    u32 mat_key = style.Material[type_index];

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
    
    PShape* shape = thing->GetPShape();
    if (shape != NULL)
        shape->MMaterial = LoadResourceByKey<RMaterial>(mat_key, 0, STREAM_PRIORITY_DEFAULT);
}

s32 GetExplosiveStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gExplosiveStyles); ++i)
    for (int j = 0; j < NUM_EXPLOSIVE_STYLES; ++j)
    {
        if (gExplosiveStyles[i].Mesh[j] == guid)
            return i;
    }

    return 0;
}

bool IsExplosiveMesh(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return false;

    for (int i = 0; i < ARRAY_LENGTH(gExplosiveStyles); ++i)
    for (int j = 0; j < NUM_EXPLOSIVE_STYLES; ++j)
    {
        if (gExplosiveStyles[i].Mesh[j] == guid)
            return true;
    }

    return false;
}

bool IsTweakExplosiveScriptAvailable()
{
    if (gTweakExplosiveScript.GetRef() == NULL)
    {
        *((CP<RScript>*)&gTweakExplosiveScript) = LoadResourceByKey<RScript>(E_TWEAK_EXPLOSIVE_SCRIPT, 0, STREAM_PRIORITY_DEFAULT);
        gTweakExplosiveScript->BlockUntilLoaded();
    }
    
    return gTweakExplosiveScript->IsLoaded();
}

void SetLevelKeyStyle(CThing* thing, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    u32 mesh_key = gLevelKeyStyles[style_index];

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
}

s32 GetLevelKeyStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gLevelKeyStyles); ++i)
    {
        if (gLevelKeyStyles[i] == guid)
            return i;
    }

    return 0;
}

void SetMagicEyeStyle(CThing* thing, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    u32 mesh_key = gMagicEyeStyles[style_index];

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
}

s32 GetMagicEyeStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gMagicEyeStyles); ++i)
    {
        if (gMagicEyeStyles[i] == guid)
            return i;
    }

    return 0;
}

void SetCreatureBrainStyle(CThing* thing, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    u32 mesh_key = gCreatureBrainStyles[style_index].Mesh;
    u32 mat_key = gCreatureBrainStyles[style_index].Material;
    u32 sound_enum = gCreatureBrainStyles[style_index].SoundEnum;

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
    
    PShape* shape = thing->GetPShape();
    if (shape != NULL)
    {
        shape->MMaterial = LoadResourceByKey<RMaterial>(mat_key, 0, STREAM_PRIORITY_DEFAULT);
        shape->SoundEnumOverride = sound_enum;
    }
}

s32 GetCreatureBrainStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gCreatureBrainStyles); ++i)
    {
        if (gCreatureBrainStyles[i].Mesh == guid)
            return i;
    }

    return 0;
}

bool IsMiscMesh(CThing* thing, s32 mesh_type)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return false;

    switch (mesh_type)
    {
        case MISC_MESH_MAGIC_EYE:
            for (int i = 0; i < ARRAY_LENGTH(gMagicEyeStyles); ++i)
            for (int j = 0; j < NUM_MAGIC_EYE_STYLES; ++j)
            {
                if (gMagicEyeStyles[i] == guid)
                    return true;
            }
            break;
            
        case MISC_MESH_SPIKE_PLATE:
            for (int i = 0; i < ARRAY_LENGTH(gSpikePlateStyles); ++i)
            for (int j = 0; j < NUM_SPIKE_PLATE_STYLES; ++j)
            {
                if (gSpikePlateStyles[i].Mesh[j] == guid)
                    return true;
            }
            break;
        
        default:
            break;
    }

    return false;
}

bool IsTweakMagicEyeScriptAvailable()
{
    if (gTweakMagicEyeScript.GetRef() == NULL)
    {
        *((CP<RScript>*)&gTweakMagicEyeScript) = LoadResourceByKey<RScript>(E_TWEAK_MAGIC_EYE_SCRIPT, 0, STREAM_PRIORITY_DEFAULT);
        gTweakMagicEyeScript->BlockUntilLoaded();
    }
    
    return gTweakMagicEyeScript->IsLoaded();
}

bool IsTweakSpikePlateScriptAvailable()
{
    if (gTweakSpikePlateScript.GetRef() == NULL)
    {
        *((CP<RScript>*)&gTweakSpikePlateScript) = LoadResourceByKey<RScript>(E_TWEAK_SPIKE_PLATE_SCRIPT, 0, STREAM_PRIORITY_DEFAULT);
        gTweakSpikePlateScript->BlockUntilLoaded();
    }
    
    return gTweakSpikePlateScript->IsLoaded();
}

void SetLeverSwitchStyle(CThing* thing, s32 type_index, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    SLeverSwitchStyle& style = gLeverSwitchStyles[style_index];
    u32 mesh_key = style.Mesh[type_index];
    u32 sound_enum = style.SoundEnum;

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
    
    PShape* shape = thing->GetPShape();
    if (shape != NULL)
        shape->SoundEnumOverride = sound_enum;
}

s32 GetLeverSwitchStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gLeverSwitchStyles); ++i)
    for (int j = 0; j < NUM_LEVER_SWITCH_STYLES; ++j)
    {
        if (gLeverSwitchStyles[i].Mesh[j] == guid)
            return i;
    }

    return 0;
}

s32 GetLeverSwitchType(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gLeverSwitchStyles); ++i)
    for (int j = 0; j < NUM_LEVER_SWITCH_TYPES; ++j)
    {
        if (gLeverSwitchStyles[i].Mesh[j] == guid)
            return j;
    }

    return LEVER_SWITCH_TRINARY;
}

void SetBouncePadStyle(CThing* thing, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    SBouncePadStyle& style = gBouncePadStyles[style_index];
    u32 mesh_key = style.Mesh;
    u32 sound_enum = style.SoundEnum;

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
    
    PShape* shape = thing->GetPShape();
    if (shape != NULL)
        shape->SoundEnumOverride = sound_enum;
}

s32 GetBouncePadStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gLeverSwitchStyles); ++i)
    for (int j = 0; j < NUM_BOUNCE_PAD_STYLES; ++j)
    {
        if (gBouncePadStyles[i].Mesh == guid)
            return i;
    }

    return 0;
}

void SetSpikePlateStyle(CThing* thing, s32 type_index, s32 style_index)
{
    if (thing == NULL) return;
    PRenderMesh* mesh = thing->GetPRenderMesh();
    
    SSpikePlateStyle& style = gSpikePlateStyles[style_index];
    u32 mesh_key = style.Mesh[type_index];
    u32 sound_enum = style.SoundEnum;

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
    
    PShape* shape = thing->GetPShape();
    if (shape != NULL)
        shape->SoundEnumOverride = sound_enum;
}

s32 GetSpikePlateStyle(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gSpikePlateStyles); ++i)
    for (int j = 0; j < NUM_SPIKE_PLATE_STYLES; ++j)
    {
        if (gSpikePlateStyles[i].Mesh[j] == guid)
            return i;
    }

    return 0;
}

s32 GetSpikePlateType(CThing* thing)
{
    CGUID guid = GetMeshGUID(thing);
    if (!guid) return 0;

    for (int i = 0; i < ARRAY_LENGTH(gSpikePlateStyles); ++i)
    for (int j = 0; j < NUM_SPIKE_PLATE_TYPES; ++j)
    {
        if (gSpikePlateStyles[i].Mesh[j] == guid)
            return j;
    }

    return SPIKE_PLATE_LARGE;
}

bool IsTweakCheckpointScriptAvailable()
{
    if (gTweakCheckpointScript.GetRef() == NULL)
    {
        *((CP<RScript>*)&gTweakCheckpointScript) = LoadResourceByKey<RScript>(E_TWEAK_CHECKPOINT_SCRIPT, 0, STREAM_PRIORITY_DEFAULT);
        gTweakCheckpointScript->BlockUntilLoaded();
    }
    
    return gTweakCheckpointScript->IsLoaded();
}

bool IsTweakShapeScriptAvailable()
{
    if (gTweakShapeScript.GetRef() == NULL)
    {
        *((CP<RScript>*)&gTweakShapeScript) = LoadResourceByKey<RScript>(E_TWEAK_SHAPE_SCRIPT, 0, STREAM_PRIORITY_DEFAULT);
        gTweakShapeScript->BlockUntilLoaded();
    }
    
    return gTweakShapeScript->IsLoaded();
}

bool ShouldAttachShapeTweak(CThing* thing)
{
    if (thing == NULL || thing->GetPScript() != NULL) return false;

    if (thing->GetPGeneratedMesh() != NULL)
        return thing->GetPShape() != NULL;

    return thing->GetPRenderMesh() != NULL;
}

MH_DefineFunc(CPoppetGooey_CanTweak, 0x0037f694, TOC1, bool, CPoppetChild*, CThing*);
bool CanTweakThing(CPoppet* poppet, CThing* thing)
{
    if (thing == NULL) return false;
    if (poppet->GetSubMode() == SUBMODE_EDIT_VERTS) return false;
    
    CPoppetChild* gooey = (CPoppetChild*)((char*)poppet + 0xa60);
    if (CPoppetGooey_CanTweak(gooey, thing)) return true;

    if (IsCheckpointMesh(thing)) return IsTweakCheckpointScriptAvailable();
    if (IsExplosiveMesh(thing)) return IsTweakExplosiveScriptAvailable();
    if (IsMiscMesh(thing, MISC_MESH_MAGIC_EYE)) return IsTweakMagicEyeScriptAvailable();
    if (IsMiscMesh(thing, MISC_MESH_SPIKE_PLATE)) return IsTweakMagicEyeScriptAvailable();

    if (thing->GetPRenderMesh() == NULL && thing->GetPGeneratedMesh() == NULL) return false;

    return IsTweakShapeScriptAvailable();
}

void OnStartTweaking(CThing* thing)
{
    StaticCP<RScript> set_script;
    if(IsCheckpointMesh(thing))
        set_script = gTweakCheckpointScript;
    else if(IsExplosiveMesh(thing))
        set_script = gTweakExplosiveScript;
    else if(IsMiscMesh(thing, MISC_MESH_MAGIC_EYE))
        set_script = gTweakMagicEyeScript;
    else if(IsMiscMesh(thing, MISC_MESH_SPIKE_PLATE))
        set_script = gTweakSpikePlateScript;
    else if(ShouldAttachShapeTweak(thing))
        set_script = gTweakShapeScript;
    else
        return;
    if (thing->GetPScript() == NULL)
        thing->AddPart(PART_TYPE_SCRIPT);
    PScript* script = thing->GetPScript();
    script->SetScript(set_script);
}

void OnStopTweaking(CThing* thing)
{
    if (thing == NULL) return;
    PScript* part = thing->GetPScript();
    if (part == NULL) return;
    CP<RScript>& script = part->ScriptInstance.Script;
    if (script.GetRef() == NULL || !script->IsLoaded()) return;
    if (script->GetGUID() == E_TWEAK_SHAPE_SCRIPT || 
        script->GetGUID() == E_TWEAK_CHECKPOINT_SCRIPT || 
        script->GetGUID() == E_TWEAK_EXPLOSIVE_SCRIPT || 
        script->GetGUID() == E_TWEAK_MAGIC_EYE_SCRIPT)
        thing->RemovePart(PART_TYPE_SCRIPT);
}

namespace TweakShapeNativeFunctions
{
    CP<CResource> GetPlan(CThing* thing)
    {
        if (thing == NULL) return NULL;

        CGUID guid = thing->PlanGUID;

        PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
        if (mesh != NULL) guid = mesh->PlanGUID;
        else if (!guid)
        {
            PGroup* group = thing->GetPGroup();
            if (group == NULL && thing->GroupHead != NULL) 
                group = thing->GroupHead->GetPGroup();

            if (group != NULL)
                guid = group->PlanDescriptor.GetGUID();
        }

        if (!guid) return NULL;

        return (CP<CResource>)LoadResourceByKey<RPlan>(guid.guid, 0, STREAM_PRIORITY_DEFAULT);
    }

    u32 GetDisplayName(CThing* thing)
    {
        u32 key = 0;

        CP<CResource> resource = GetPlan(thing);
        if (resource && resource->IsLoaded() && resource->GetResourceType() == RTYPE_PLAN)
            key = ((RPlan*)resource.GetRef())->InventoryData.NameTranslationTag;
        
        if (thing != NULL && key == 0)
        {
            if (thing->GetPRenderMesh() != NULL)
                key = thing->GetPShape() != NULL ? E_LAMS_TWEAKABLE_MESH : E_LAMS_TWEAKABLE_DECAL;
            else
                key = E_LAMS_TWEAKABLE_MATERIAL;
        }

        return key; 
    }

    bool UsesParameterAnimations(CThing* thing)
    {
        if (thing == NULL) return false;
        PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
        if (mesh == NULL) return false;
        CP<RGfxMaterial>& gmat = mesh->GfxMaterial;
        if (!gmat || !gmat->IsLoaded()) return false;
        return gmat->ParameterAnimations.size() != 0;
    }

    bool UsesPlayerDefinedColour(CThing* thing)
    {
        if (thing == NULL) return false;

        PShape* shape = thing->GetPShape();
        if (shape != NULL && shape->LethalType == LETHAL_BULLET)
            return true;

        PGeneratedMesh* generated_mesh = thing->GetPGeneratedMesh();
        if (generated_mesh != NULL)
        {
            CP<RGfxMaterial>& gmat = generated_mesh->GfxMaterial;
            if (!gmat || !gmat->IsLoaded()) return false;
            return gmat->UsesPlayerDefinedColour;
        }

        PRenderMesh* render_mesh = thing->GetPRenderMesh();
        if (render_mesh != NULL)
        {
            CP<RMesh>& mesh = render_mesh->Mesh;
            if (!mesh || !mesh->IsLoaded()) return false;
            CVector<CPrimitive>& primitives = mesh->mesh.Primitives;
            for (CPrimitive* it = primitives.begin(); it != primitives.end(); ++it)
            {
                CPrimitive& primitive = *it;
                CP<RGfxMaterial>& gmat = primitive.Material;
                if (!gmat || !gmat->IsLoaded()) continue;
                if (gmat->UsesPlayerDefinedColour) return true;
            }

            return false;
        }

        return false;
    }
    
    bool UsesMeshAnimations(CThing* thing)
    {
        if (thing == NULL) return false;
        PRenderMesh* mesh = thing->GetPRenderMesh();
        if (mesh == NULL) return false;
        CP<RAnim>& anim = mesh->Anim;
        if (!anim) return false;
        return true;
    }

    void PerformToolAction(CThing* thing, CScriptObjectPoppet* so_poppet, EToolType tool)
    {
        if (thing == NULL || so_poppet == NULL) return;
        CPoppet* poppet = so_poppet->GetNativeObject();
        if (poppet == NULL) return;

        switch (tool)
        {
            case TOOL_SHAPE_UNLETHAL:
            {
                poppet->DangerMode = LETHAL_NOT;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_PLASMA:
            {
                poppet->DangerMode = LETHAL_BULLET;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_ELECTRIFY:
            {
                poppet->DangerMode = LETHAL_ELECTRIC;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_BURNINATE:
            {
                poppet->DangerMode = LETHAL_FIRE;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_ICE:
            {
                poppet->DangerMode = LETHAL_ICE;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_GAS:
            {
                ELethalType lethal_type = thing->GetPShape()->LethalType;
                if(lethal_type > 6 && lethal_type < 12) { poppet->DangerMode = lethal_type; }
                else { poppet->DangerMode = LETHAL_POISON_GAS; }
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_SPIKE:
            {
                poppet->DangerMode = LETHAL_SPIKE;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_DROWNED:
            {
                poppet->DangerMode = LETHAL_DROWNED;
                poppet->SetDangerType(thing);
                break;   
            }
            case TOOL_SHAPE_CRUSH:
            {
                poppet->DangerMode = LETHAL_CRUSH;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_EYEDROPPER:
            {
                if(thing->GetPGeneratedMesh())
                    poppet->EyedropperPick(thing);
                break;
            }
            case TOOL_SHAPE_FLOOD_FILL:
            {
                if(thing->GetPGeneratedMesh())
                    poppet->PushMode(MODE_MENU, SUBMODE_CHOOSE_MATERIAL);
                break;
            }
            case TOOL_SHAPE_VERTEX_EDIT:
            {
                if(thing->GetPShape())
                    poppet->PushMode(MODE_EDIT, SUBMODE_EDIT_VERTS);
                break;
            }
            case TOOL_SHAPE_TAKE_PLAN:
            {
                CVector<CThingPtr> things;
                things.push_back(thing);
                poppet->Inventory.TakePlan(things);
                break;
            }
        }
    }

    float GetTextureAnimationSpeed(CThing* thing)
    {
        PGeneratedMesh* mesh;
        if (thing == NULL || (mesh = thing->GetPGeneratedMesh()) == NULL) return 0.0f;
        return mesh->TextureAnimationSpeed;
    }

    void Register()
    {
        RegisterNativeFunction("TweakShape", "GetDisplayName__Q5Thing", true, NVirtualMachine::CNativeFunction1<u32, CThing*>::Call<GetDisplayName>);
        RegisterNativeFunction("TweakShape", "GetPlan__Q5Thing", true, NVirtualMachine::CNativeFunction1<CP<CResource>, CThing*>::Call<GetPlan>);

        RegisterNativeFunction("TweakShape", "UsesPlayerDefinedColour__Q5Thing", true, NVirtualMachine::CNativeFunction1<bool, CThing*>::Call<UsesPlayerDefinedColour>);
        RegisterNativeFunction("TweakShape", "UsesParameterAnimations__Q5Thing", true, NVirtualMachine::CNativeFunction1<bool, CThing*>::Call<UsesParameterAnimations>);
        RegisterNativeFunction("TweakShape", "UsesMeshAnimations__Q5Thing", true, NVirtualMachine::CNativeFunction1<bool, CThing*>::Call<UsesMeshAnimations>);
        RegisterNativeFunction("TweakShape", "PerformToolAction__Q5ThingQ6Poppeti", true, NVirtualMachine::CNativeFunction3V<CThing*, CScriptObjectPoppet*, EToolType>::Call<PerformToolAction>);
        RegisterNativeFunction("TweakShape", "GetTextureAnimationSpeed__Q5Thing", true, NVirtualMachine::CNativeFunction1<float, CThing*>::Call<GetTextureAnimationSpeed>);
    }
}