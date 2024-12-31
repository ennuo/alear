#include <cell/gcm.h>

#include <cell/DebugLog.h>

#include "thing.h"

#include "ResourceGfxMaterial.h"
#include "PartYellowHead.h"

#include "SceneGraph.h"
#include "GFXApi.h"
#include "Clock.h"
#include "Colour.h"

void OnGfxMaterialBinded(RGfxMaterial* gmat, CGprogram program, u32 offset, CMeshInstance* instance)
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
                    cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, (float*)&anim->ExpressionState, offset);
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
                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, primary_v4, offset);
            
            parameter = cellGcmCgGetNamedParameter(program, "iPlayerColour1");
            if (parameter != NULL)
                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, secondary_v4, offset);

            parameter = cellGcmCgGetNamedParameter(program, "iPlayerColour2");
            if (parameter != NULL)
                cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, tertiary_v4, offset);
        }
    }

    parameter = cellGcmCgGetNamedParameter(program, "iTime");
    if (parameter != NULL)
    {
        float time = GetClockSeconds();
        cellGcmSetFragmentProgramParameter(gCellGcmCurrentContext, program, parameter, &time, offset);
    }
}