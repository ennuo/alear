#include <cell/gcm.h>

#include <cell/DebugLog.h>

#include "thing.h"

#include "ResourceGfxMaterial.h"
#include "PartYellowHead.h"

#include "SceneGraph.h"
#include "GFXApi.h"
#include "Clock.h"
#include "Colour.h"

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

    parameter = cellGcmCgGetNamedParameter(program, "iTime");
    if (parameter != NULL)
    {
        float time = GetClockSeconds();
        cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, &time, ucode_offset);
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

        float key[4];
        Vectormath::Aos::storeXYZW(anim.BaseValue, key);

        int num_components = __builtin_popcount(anim.ComponentsAnimated);
        int stride = anim.Keys.size() / num_components;
        int frame = gGraphicsFrameNum % stride;

        int offset = 0;
        v4 value = anim.BaseValue;
        for (int j = 0; j < 4; ++j)
        {
            if ((anim.ComponentsAnimated & (1 << j)) == 0) continue;
            // value = value.setElem(j, anim.Keys[offset + frame]);

            key[j] = anim.Keys[offset + frame];

            offset += stride;
        }

        // DebugLog("<%f, %f, %f, %f> : (%d/%d)\n", key[0], key[1], key[2], key[3], frame, stride);
        cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, param, key, ucode_offset);
    }
}