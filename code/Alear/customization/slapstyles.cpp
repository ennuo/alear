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
        DebugLog("CSlapMesh:\n\tSound: %s\n\tGUID: %08x\n\tLeftHand: %s\n\tRightHand: %s\n\tKill: %s\n\tStrength: %f\n",
            slap_mesh.Sound.c_str(),
            slap_mesh.GUID, 
            slap_mesh.LeftHand ? "true" : "false", 
            slap_mesh.RightHand ? "true" : "false",
            slap_mesh.Kill ? "true" : "false",
            slap_mesh.Strength);
    }

    return true;
}

void UnloadSlapStyles()
{
    gSlapMeshes.clear();
}

void OnPlayerSlapped(CThing* victim, CThing* by, v2 force, v2 const* pos3d, bool left_hand)
{
    bool right_hand = !left_hand;
    float strength = 1.0f;

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
                strength = mesh.Strength;

                if (mesh.Kill)
                    victim->GetPCreature()->SetState(STATE_DEAD);

                break;
            }
        }
    }

    victim->GetPCreature()->BeSlapped(by, force * strength);
    CAudio::PlaySample(CAudio::gSFX, sound, 1.0f, pos3d, -10000.0f);
}