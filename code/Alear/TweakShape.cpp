#include <thing.h>
#include <refcount.h>
#include <Poppet.h>
#include <ResourceSystem.h>
#include <ResourceScript.h>
#include <PartGeneratedMesh.h>
#include <PartScript.h>

#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptFunction.h>

#include <poppet/ScriptObjectPoppet.h>

const u32 E_TWEAK_SHAPE_SCRIPT = 3796510132u;

StaticCP<RScript> gTweakShapeScript;

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
        thing->GetPRenderMesh() != NULL || 
        thing->GetPScript() != NULL || 
        thing->GetPGeneratedMesh() == NULL) return false;
    
    PShape* shape = thing->GetPShape();
    return shape != NULL; // && (shape->LethalType < LETHAL_POISON_GAS || shape->LethalType > LETHAL_POISON_GAS6);
}

MH_DefineFunc(CPoppetGooey_CanTweak, 0x0037f694, TOC1, bool, CPoppetChild*, CThing*);
bool CanTweakThing(CPoppet* poppet, CThing* thing)
{
    if (thing == NULL) return false;
    
    CPoppetChild* gooey = (CPoppetChild*)((char*)poppet + 0xa60);
    if (CPoppetGooey_CanTweak(gooey, thing)) return true;

    if (thing->GetPRenderMesh() != NULL) return false;
    if (thing->GetPGeneratedMesh() == NULL) return false;

    PShape* shape = thing->GetPShape();
    if (shape->LethalType >= LETHAL_POISON_GAS && shape->LethalType <= LETHAL_POISON_GAS6)
        return true;

    return IsTweakShapeScriptAvailable();
}

void OnStartTweaking(CThing* thing)
{
    if (!ShouldAttachShapeTweak(thing)) return;

    if (thing->GetPScript() == NULL)
        thing->AddPart(PART_TYPE_SCRIPT);
    
    PScript* script = thing->GetPScript();
    script->SetScript(gTweakShapeScript);
}

void OnStopTweaking(CThing* thing)
{
    if (thing == NULL) return;
    PScript* part = thing->GetPScript();
    if (part == NULL) return;
    CP<RScript>& script = part->ScriptInstance.Script;
    if (script.GetRef() == NULL || !script->IsLoaded()) return;
    if (script->GetGUID() == E_TWEAK_SHAPE_SCRIPT)
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
        PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
        if (mesh == NULL || !mesh->PlanGUID) return NULL;
        return (CP<CResource>)LoadResourceByKey<RPlan>(mesh->PlanGUID.guid, 0, STREAM_PRIORITY_DEFAULT);
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

        PGeneratedMesh* mesh = thing->GetPGeneratedMesh();
        if (mesh == NULL) return false;
        CP<RGfxMaterial>& gmat = mesh->GfxMaterial;
        if (!gmat || !gmat->IsLoaded()) return false;
        return gmat->UsesPlayerDefinedColour;
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