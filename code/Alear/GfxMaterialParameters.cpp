#include <cell/gcm.h>

#include <cell/DebugLog.h>

#include "thing.h"

#include "ResourceGfxMaterial.h"
#include "PartYellowHead.h"

#include "SceneGraph.h"
#include "GFXApi.h"
#include "Clock.h"
#include "Colour.h"

#include <mmalex.h>

void OnGfxMaterialBinded(RGfxMaterial* gmat, CGprogram program, u32 ucode_offset, CMeshInstance* instance)
{
    CGparameter parameter;

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
                parameter = cellGcmCgGetNamedParameter(program, "iExpressionLevel");
                if (parameter != NULL)
                    cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, (float*)&anim->ExpressionState, ucode_offset);
            };

            c32 primary = yellowhead->GetColour(PLAYER_COLOUR_PRIMARY);
            c32 secondary = yellowhead->GetColour(PLAYER_COLOUR_SECONDARY);
            c32 tertiary = yellowhead->GetColour(PLAYER_COLOUR_TERTIARY);

            float primary_v4[4];
            float secondary_v4[4];
            float tertiary_v4[4];

            primary.GetRGBAf(primary_v4);
            secondary.GetRGBAf(secondary_v4);
            tertiary.GetRGBAf(tertiary_v4);

            parameter = cellGcmCgGetNamedParameter(program, "iPlayerColour0");
            if (parameter != NULL)
                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, primary_v4, ucode_offset);
            
            parameter = cellGcmCgGetNamedParameter(program, "iPlayerColour1");
            if (parameter != NULL)
                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, secondary_v4, ucode_offset);

            parameter = cellGcmCgGetNamedParameter(program, "iPlayerColour2");
            if (parameter != NULL)
                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, tertiary_v4, ucode_offset);
        }
    }



    const float DELTA_TIME = 1.0f / 30.0f;
    float time = GetClockSeconds();

    parameter = cellGcmCgGetNamedParameter(program, "iTime");
    if (parameter != NULL)
    {
        cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, &time, ucode_offset);
    }


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

        char name[4];
        name[0] = 't';
        name[1] = anim.Name[0];
        name[2] = anim.Name[1];
        name[3] = '\0';

        CGparameter param = cellGcmCgGetNamedParameter(program, name);

        if (param == NULL || anim.Keys.size() == 0) continue;

        v4 last_key = anim.BaseValue;
        v4 next_key = anim.BaseValue;

        // float last_key[4];
        // float next_key[4];
        // float key[4];

        // Vectormath::Aos::storeXYZW(anim.BaseValue, last_key);
        // Vectormath::Aos::storeXYZW(anim.BaseValue, next_key);

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