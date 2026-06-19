#include <cell/gcm.h>

#include <cell/DebugLog.h>

#include "thing.h"

#include "ResourceGfxMaterial.h"
#include "PartYellowHead.h"

#include "SceneGraph.h"
#include "GFXApi.h"
#include "Clock.h"
#include "Colour.h"

#include <ResourceGame.h>
#include <Player.h>

#include <mmalex.h>

static float gLethalRemap[LETHAL_TYPE_COUNT] =
{
    0.0f, // LETHAL_NOT
    1.0f / 8.0f, // LETHAL_FIRE
    2.0f / 8.0f, // LETHAL_ELECTRIC
    3.0f / 8.0f, // LETHAL_ICE
    4.0f / 8.0f, // LETHAL_CRUSH
    5.0f / 8.0f, // LETHAL_SPIKE
    6.0f / 8.0f, // LETHAL_POISON_GAS
    6.0f / 8.0f, // LETHAL_POISON_GAS2
    6.0f / 8.0f, // LETHAL_POISON_GAS3
    6.0f / 8.0f, // LETHAL_POISON_GAS4
    6.0f / 8.0f, // LETHAL_POISON_GAS5
    6.0f / 8.0f, // LETHAL_POISON_GAS6
    0.0f, // LETHAL_NO_STAND
    2.0f / 8.0f, // LETHAL_BULLET
    7.0f / 8.0f, // LETHAL_DROWNED
};

void OnGfxMaterialBinded(RGfxMaterial* gmat, CGprogram program, u32 shader, u32 ucode_offset, CMeshInstance* instance)
{
    shader = shader >> 2;
    gmat->CacheParameters();

    const RGfxMaterial::UniformCache& c = gmat->CachedUniforms[shader];

    if (instance != NULL && c.EditorColourAlpha != NULL)
    {
        // lbp1 fucks up the alpha of the editor color in the vertex shader
        // uThingCol.w = frac(0.125 * uThingCol.w)
        float w = instance->InstanceColor.getW().getAsFloat();
        cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, c.EditorColourAlpha, &w, ucode_offset);
    }

    // If we actually have an instance pointer,
    // try to bind the expression/time parameters.
    // Should maybe add flags for whether or not a function has time/expression
    // constants to save on constant binding calls?
    if (instance != NULL && instance->MyThing != NULL)
    {
        PYellowHead* yellowhead = instance->MyThing->GetPYellowHead();
        if (yellowhead != NULL && yellowhead->RenderYellowHead != NULL)
        {
            CSackBoyAnim* anim = yellowhead->RenderYellowHead->SackBoyAnim;
            if (anim != NULL)
            {
                if (c.ExpressionLevel != NULL)
                    cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, c.ExpressionLevel, (float*)&anim->ExpressionState, ucode_offset);
            };

            for (u32 i = 0; i < 3; ++i)
            {
                if (c.PlayerColour[i] == NULL) continue;

                float colour[4];
                yellowhead->GetColour((EPlayerColour)i).GetRGBAf(colour);

                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, c.PlayerColour[i], colour, ucode_offset);
            }

            if (c.PlayerNumber != NULL)
            {

                int index = 0;
                for (u32 i = 0; i < 4; ++i)
                {
                    CPlayer* player = gGame->GetPlayerFromIndex(i);
                    if (player != NULL && player->PlayerNumber == yellowhead->PlayerNumber)
                    {
                        index = i;
                        break;
                    }
                }

                float indexf = index / 4.0f;
                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, c.PlayerNumber, &indexf, ucode_offset);
            }
        }
        
        PCreature* creature = instance->MyThing->GetPCreature();
        if (creature != NULL && c.DeathParams != NULL)
        {
            int lethal = creature->GetTypeOfLethalThingTouched();
            bool dead = creature->GetState() == STATE_DEAD;
            float params[4] = { gLethalRemap[lethal >= 0 && lethal < LETHAL_TYPE_COUNT ? lethal : 0], dead ? 0.5f : 0.0f, dead ? creature->GetStateTimer() / 30.0f : 0.0f, 0.0f };
            cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, c.DeathParams, params, ucode_offset);
        }

    }



    const float DELTA_TIME = 1.0f / 30.0f;
    float time = GetClockSeconds();
    if (c.Time != NULL)
        cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, c.Time, &time, ucode_offset);


    if (gmat->ParameterAnimations.size() == 0) return;

    bool lerp = false;
    float anim_speed = 1.0f;
    if (instance != NULL && instance->MyThing != NULL)
    {
        PGeneratedMesh* mesh = instance->MyThing->GetPGeneratedMesh();
        if (mesh != NULL)
        {
            lerp = true;
            anim_speed = mesh->TextureAnimationSpeed;
        }
    }

    for (int i = 0; i < gmat->ParameterAnimations.size(); ++i)
    {
        CMaterialParameterAnimation& anim = gmat->ParameterAnimations[i];
        CGparameter param = anim.ParameterCache[shader];
        if (param == NULL || anim.Keys.size() == 0) continue;

        v4 last_key = anim.BaseValue;
        v4 next_key = anim.BaseValue;

        int num_components = __builtin_popcount(anim.ComponentsAnimated);
        int stride = anim.Keys.size() / num_components;

        float factor;
        int frame, next_frame;

        if (lerp)
        {
            int num_keys = anim.Keys.size() - 1;
            float duration = DELTA_TIME * num_keys;

            float pos = mmalex::fmod(time * anim_speed, duration);
            float pct = pos / duration;

            frame = (int)(pct * num_keys);
            next_frame = frame + 1;
            
            float last_pos = frame * DELTA_TIME;
            float next_pos = (frame + 1) * DELTA_TIME;

            factor = (pos - last_pos) / (next_pos - last_pos);
        }
        else
        {
            frame = gGraphicsFrameNum % stride;
            next_frame = (frame + 1) % stride;
            factor = 0.0f;
        }
        
        int offset = 0;
        for (int j = 0; j < 4; ++j)
        {
            if ((anim.ComponentsAnimated & (1 << j)) == 0) continue;

            last_key.setElem(j, anim.Keys[offset + frame]);
            next_key.setElem(j, anim.Keys[offset + next_frame]);

            offset += stride;
        }

        v4 key = Vectormath::Aos::lerp(factor, last_key, next_key);
        
        cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, param, (float*)&key, ucode_offset);
    }
}

bool HandleShadowDrawCall(CMeshInstance* instance)
{
    CMesh* mesh = instance->Mesh;
    if (mesh == NULL) return false;

    PCostume* costume;
    if (instance->MyThing && (costume = instance->MyThing->GetPCostume()) != NULL)
    {
        if (instance->InstancePrimitives == NULL) return false;

        for (u32 i = 0; i < instance->InstancePrimitives->size(); ++i)
        {
            const CPrimitive& p = (*instance->InstancePrimitives)[i];
            cellGcmSetDrawIndexArray(gCellGcmCurrentContext, 
                mesh->PrimitiveType, p.NumIndices, CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16,
                mesh->Indices.GetLocation(),
                mesh->Indices.GetOffset() + (p.FirstIndex * sizeof(u16))
            );
        }
        
        return true;
    }

    return false;
}

void RGfxMaterial::CacheParameters()
{
    if (UniformsCached) return;

    UniformsCached = true;
    UsesPlayerDefinedColour = false;

    CellCgbFragmentProgramConfiguration conf;
    memset(&conf, 0, sizeof(CellCgbFragmentProgramConfiguration));
    if (cellGcmCgGetCgbFragmentProgramConfiguration(Shaders[SHADER_C], &conf, CELL_GCM_FALSE, 1, 0) == CELL_OK)
    {
        if ((conf.attributeInputMask & CELL_GCM_ATTRIB_OUTPUT_MASK_FRONTDIFFUSE))
            UsesPlayerDefinedColour = true;
    }

    for (u32 i = 0; i < SHADER_LAST; ++i)
    {
        UniformCache& c = CachedUniforms[i];
        CGprogram program = Shaders[i];
        
        c.Time = cellGcmCgGetNamedParameter(program, "iTime");
        c.ExpressionLevel = cellGcmCgGetNamedParameter(program, "iExpressionLevel");
        c.PlayerNumber = cellGcmCgGetNamedParameter(program, "iPlayerNumber");
        c.DeathParams = cellGcmCgGetNamedParameter(program, "iDeathParams");
        c.EditorColourAlpha = cellGcmCgGetNamedParameter(program, "iEditorColourAlpha");

        if (c.Time != NULL)
            UsesTime = true;

        for (u32 j = 0; j < 3; ++j)
        {
            char name[16] = { 0 };
            sprintf(name, "iPlayerColour%d", j);
            c.PlayerColour[j] = cellGcmCgGetNamedParameter(program, name);
        }

        for (u32 j = 0; j < ParameterAnimations.size(); ++j)
        {
            CMaterialParameterAnimation& anim = ParameterAnimations[j];

            char name[4];
            name[0] = 't';
            name[1] = anim.Name[0];
            name[2] = anim.Name[1];
            name[3] = '\0';

            anim.ParameterCache[i] = cellGcmCgGetNamedParameter(program, name);
        }
    }
    
}