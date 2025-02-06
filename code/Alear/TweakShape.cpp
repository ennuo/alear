#include <thing.h>
#include <refcount.h>
#include <Poppet.h>
#include <ResourceSystem.h>
#include <ResourceScript.h>
#include <ResourceGFXMesh.h>
#include <PartGeneratedMesh.h>
#include <PartRenderMesh.h>
#include <PartScript.h>

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

#include <poppet/ScriptObjectPoppet.h>

#include "CheckpointStyles.h"

const u32 E_TWEAK_SHAPE_SCRIPT = 3796510132u;
const u32 E_TWEAK_CHECKPOINT_SCRIPT = 4286805021u;

StaticCP<RScript> gTweakShapeScript;
StaticCP<RScript> gTweakCheckpointScript;



struct SCheckpointStyle {
    CGUID Mesh[NUM_CHECKPOINT_TYPES];
    CGUID Material;
};

SCheckpointStyle gCheckpointStyles[] =
{
    // Cardboard
    {
        { 122175, 120921, 120915, 120935 },
        10724
    },

    // Wood
    {
        { 31238, 11668, 55455, 68386 },
        10717
    },

    // Plastic
    {
        { 119228, 119230, 119227, 120939 },
        10718
    },

    // Chrome
    {
        { 119221, 119217, 119220, 120941 },
        10716
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
    u32 mat_key = style.Material;

    if (mesh != NULL)
        mesh->Mesh = LoadResourceByKey<RMesh>(mesh_key, 0, STREAM_PRIORITY_DEFAULT);
    
    PShape* shape = thing->GetPShape();
    if (shape != NULL)
        shape->MMaterial = LoadResourceByKey<RMaterial>(mat_key, 0, STREAM_PRIORITY_DEFAULT);

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
    for (int j = 0; j < NUM_CHECKPOINT_TYPES; ++j)
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
    if (thing == NULL || 
        (thing->GetPRenderMesh() == NULL && thing->GetPGeneratedMesh() == NULL) ||
        thing->GetPScript() != NULL) return false;
    
    PShape* shape = thing->GetPShape();
    return shape != NULL; // && (shape->LethalType < LETHAL_POISON_GAS || shape->LethalType > LETHAL_POISON_GAS6);
}

MH_DefineFunc(CPoppetGooey_CanTweak, 0x0037f694, TOC1, bool, CPoppetChild*, CThing*);
bool CanTweakThing(CPoppet* poppet, CThing* thing)
{
    if (thing == NULL) return false;
    
    CPoppetChild* gooey = (CPoppetChild*)((char*)poppet + 0xa60);
    if (CPoppetGooey_CanTweak(gooey, thing)) return true;

    if (IsCheckpointMesh(thing)) return IsTweakCheckpointScriptAvailable();

    if (thing->GetPRenderMesh() == NULL && thing->GetPGeneratedMesh() == NULL) return false;

    PShape* shape = thing->GetPShape();
    if (shape == NULL) return false;

    if (shape->LethalType >= LETHAL_POISON_GAS && shape->LethalType <= LETHAL_POISON_GAS6)
        return true;

    return IsTweakShapeScriptAvailable();
}

void OnStartTweaking(CThing* thing)
{
    bool is_shape_tweak = ShouldAttachShapeTweak(thing);
    bool is_checkpoint_tweak = IsCheckpointMesh(thing);

    if (!is_shape_tweak && !is_checkpoint_tweak) return;

    if (thing->GetPScript() == NULL)
        thing->AddPart(PART_TYPE_SCRIPT);
    
    PScript* script = thing->GetPScript();
    script->SetScript(is_checkpoint_tweak ? gTweakCheckpointScript : gTweakShapeScript);
}

void OnStopTweaking(CThing* thing)
{
    if (thing == NULL) return;
    PScript* part = thing->GetPScript();
    if (part == NULL) return;
    CP<RScript>& script = part->ScriptInstance.Script;
    if (script.GetRef() == NULL || !script->IsLoaded()) return;
    if (script->GetGUID() == E_TWEAK_SHAPE_SCRIPT || script->GetGUID() == E_TWEAK_CHECKPOINT_SCRIPT)
        thing->RemovePart(PART_TYPE_SCRIPT);
}

namespace TweakShapeNativeFunctions
{
    u32 GetDisplayName(CP<CResource> resource)
    {
        if (!resource || !resource->IsLoaded() || resource->GetResourceType() != RTYPE_PLAN) return 0;
        return ((RPlan*)resource.GetRef())->InventoryData.NameTranslationTag;
    }

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
            case TOOL_SHAPE_ICE:
            {
                poppet->DangerMode = LETHAL_ICE;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_BURNINATE:
            {
                poppet->DangerMode = LETHAL_FIRE;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_CRUSH:
            {
                poppet->DangerMode = LETHAL_CRUSH;
                poppet->SetDangerType(thing);
                break;
            }
            case TOOL_SHAPE_DROWNED:
            {
                poppet->DangerMode = LETHAL_DROWNED;
                poppet->SetDangerType(thing);
                break;   
            }
            case TOOL_SHAPE_SPIKE:
            {
                poppet->DangerMode = LETHAL_SPIKE;
                poppet->SetDangerType(thing);
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
        RegisterNativeFunction("TweakShape", "GetDisplayName__Q8Resource", true, NVirtualMachine::CNativeFunction1<u32, CP<CResource> >::Call<GetDisplayName>);
        RegisterNativeFunction("TweakShape", "GetPlan__Q5Thing", true, NVirtualMachine::CNativeFunction1<CP<CResource>, CThing*>::Call<GetPlan>);

        RegisterNativeFunction("TweakShape", "UsesPlayerDefinedColour__Q5Thing", true, NVirtualMachine::CNativeFunction1<bool, CThing*>::Call<UsesPlayerDefinedColour>);
        RegisterNativeFunction("TweakShape", "UsesParameterAnimations__Q5Thing", true, NVirtualMachine::CNativeFunction1<bool, CThing*>::Call<UsesParameterAnimations>);
        RegisterNativeFunction("TweakShape", "PerformToolAction__Q5ThingQ6Poppeti", true, NVirtualMachine::CNativeFunction3V<CThing*, CScriptObjectPoppet*, EToolType>::Call<PerformToolAction>);
        RegisterNativeFunction("TweakShape", "GetTextureAnimationSpeed__Q5Thing", true, NVirtualMachine::CNativeFunction1<float, CThing*>::Call<GetTextureAnimationSpeed>);
    }
}