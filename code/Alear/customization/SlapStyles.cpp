#include "customization/SlapStyles.h"

#include <cell/DebugLog.h>
#include <refcount.h>
#include <vector.h>

#include <MMAudio.h>
#include <PartCostume.h>
#include <ResourceGFXMesh.h>
#include <Variable.h>
#include <ResourceFileOfBytes.h>
#include <ResourceSystem.h>

CVector<CSlapMesh> gSlapMeshes;
StaticCP<RFileOfBytes> gSlapStyleData;

bool LoadSlapStyles()
{
    gSlapMeshes.clear();

    CP<RFileOfBytes> file = LoadResourceByKey<RFileOfBytes>(E_SLAP_STYLES_KEY, 0, STREAM_PRIORITY_DEFAULT);
    *((CP<RFileOfBytes>*)&gSlapStyleData) = file;
    
    file->BlockUntilLoaded();
    if (!file->IsLoaded()) return false;

    ByteArray& b = file->GetData();
    CGatherVariables variables;
    variables.Init<CSlapStyles>((CSlapStyles*)&gSlapMeshes);
    if (GatherVariablesLoad(b, variables, true, NULL) != REFLECT_OK)
{
        DebugLog("An error occurred while loading data for slap styles!\n");
        return false;
    }

    CSlapMesh* it = gSlapMeshes.begin();
    for (; it != gSlapMeshes.end(); ++it)
    {
        CSlapMesh& slap_mesh = *it;
        DebugLog("CSlapMesh:\n\tSound: %s\n\tGUID: %08x\n\tLeftHand: %s\n\tRightHand: %s\n\tKill: %s\n\tHorizontal Force: %f\n\tVertical Force: %f\n",
            slap_mesh.Sound.c_str(),
            slap_mesh.GUID, 
            slap_mesh.LeftHand ? "true" : "false", 
            slap_mesh.RightHand ? "true" : "false",
            slap_mesh.Kill ? "true" : "false",
            slap_mesh.HorizontalForce, 
            slap_mesh.VerticalForce);
    }

    return true;
}

void UnloadSlapStyles()
{
    gSlapMeshes.clear();
}


static float gHorizontalSlapForce;
static float gVerticalSlapForce;
v4 GetSlapForce(v4 stick_vel)
{
    if (stick_vel.getX() < 0.0f) 
        return v4(-gHorizontalSlapForce, gVerticalSlapForce, 0.0f, 0.0f);
    return v4(gHorizontalSlapForce, gVerticalSlapForce, 0.0f, 0.0f);
}

void OnPlayerSlapped(CThing* victim, CThing* by, v2 force, v2 const* pos3d, bool left_hand)
{
    bool right_hand = !left_hand;
    
    gHorizontalSlapForce = 23.0f;
    gVerticalSlapForce = 0.0f;

    const char* sound = "character/slap";
    PCostume* costume = by->GetPCostume();
    if (costume != NULL)
    {
        CCostumePiece& hands = costume->CostumePieceVec[COSTUMEPART_HANDS];
        CP<RMesh>& hand_mesh = hands.Mesh;
        if (hand_mesh)
        {
            CSlapMesh* it = gSlapMeshes.begin();
            for (; it != gSlapMeshes.end(); ++it)
            {
                CSlapMesh& mesh = *it;

                if (hand_mesh->GetGUID().guid != mesh.GUID) continue;
                if ((left_hand && !mesh.LeftHand) || (right_hand && !mesh.RightHand)) continue;

                sound = mesh.Sound.c_str();
                gHorizontalSlapForce = mesh.HorizontalForce;
                gVerticalSlapForce = mesh.VerticalForce;

                if (mesh.Kill)
                    victim->GetPCreature()->SetState(STATE_DEAD);

                break;
            }
        }
    }

    victim->GetPCreature()->BeSlapped(by, force);
    CAudio::PlaySample(CAudio::gSFX, sound, 1.0f, pos3d, -10000.0f);
}